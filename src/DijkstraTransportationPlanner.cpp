#include "DijkstraTransportationPlanner.h"
#include "DijkstraPathRouter.h"
#include "BusSystemIndexer.h"
#include "GeographicUtils.h"
#include <unordered_map>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <queue>
#include <limits>

struct CDijkstraTransportationPlanner::SImplementation{
    using EMode = CTransportationPlanner::ETransportationMode;
    using TTripStep = CTransportationPlanner::TTripStep;
    using TNodeID = CStreetMap::TNodeID;

    std::shared_ptr<CStreetMap> DStreetMap;
    std::shared_ptr<CBusSystem> DBusSystem;
    std::shared_ptr<CBusSystemIndexer> DBusSystemIndexer;
    double DWalkSpeed;        // mph, from config
    double DBikeSpeed;        // mph, from config
    double DDefaultSpeedLimit;// mph, used when way has no maxspeed tag
    double DBusStopTime;      // seconds added per bus stop
    int DPrecomputeTime;

    // Street map nodes sorted by ID; index in this vector is the vertex ID used by routers
    std::vector<std::shared_ptr<CStreetMap::SNode>> DSortedNodes;
    std::unordered_map<TNodeID, std::size_t> DNodeIDToIndex;

    // Graph for shortest (distance) path; respects oneway restrictions
    std::shared_ptr<CDijkstraPathRouter> DShortestRouter;

    // Edge for the fastest-path adjacency list, carrying travel time and transport mode
    struct SFastEdge {
        std::size_t DDestIdx;
        double DTime;  // hours
        EMode DMode;
    };
    // Adjacency list for fastest-path Dijkstra, indexed by node index
    std::vector<std::vector<SFastEdge>> DFastAdj;

    // Returns the speed limit (mph) of the way connecting src→dst, or DDefaultSpeedLimit if not found
    double GetSpeedBetween(TNodeID src, TNodeID dst) const {
        for(std::size_t wi = 0; wi < DStreetMap->WayCount(); wi++){
            auto Way = DStreetMap->WayByIndex(wi);
            for(std::size_t ni = 1; ni < Way->NodeCount(); ni++){
                auto A = Way->GetNodeID(ni-1);
                auto B = Way->GetNodeID(ni);
                if((A == src && B == dst) || (A == dst && B == src)){
                    if(Way->HasAttribute("maxspeed")){
                        std::istringstream SS(Way->GetAttribute("maxspeed"));
                        double Val;
                        if(SS >> Val) return Val;
                    }
                    return DDefaultSpeedLimit;
                }
            }
        }
        return DDefaultSpeedLimit;
    }

    SImplementation(std::shared_ptr<SConfiguration> config){
        DStreetMap = config->StreetMap();
        DBusSystem = config->BusSystem();
        DWalkSpeed = config->WalkSpeed();
        DBikeSpeed = config->BikeSpeed();
        DDefaultSpeedLimit = config->DefaultSpeedLimit();
        DBusStopTime = config->BusStopTime();
        DPrecomputeTime = config->PrecomputeTime();

        DBusSystemIndexer = std::make_shared<CBusSystemIndexer>(DBusSystem);

        // Collect and sort nodes by ID
        for(std::size_t i = 0; i < DStreetMap->NodeCount(); i++){
            DSortedNodes.push_back(DStreetMap->NodeByIndex(i));
        }
        std::sort(DSortedNodes.begin(), DSortedNodes.end(),
            [](const std::shared_ptr<CStreetMap::SNode> &a, const std::shared_ptr<CStreetMap::SNode> &b){
                return a->ID() < b->ID();
            });

        for(std::size_t i = 0; i < DSortedNodes.size(); i++){
            DNodeIDToIndex[DSortedNodes[i]->ID()] = i;
        }

        DShortestRouter = std::make_shared<CDijkstraPathRouter>();
        DFastAdj.resize(DSortedNodes.size());

        // Add vertices to shortest router
        for(auto &Node : DSortedNodes){
            DShortestRouter->AddVertex(Node->ID());
        }

        // Helper: add a fast edge (keeps only the minimum time edge per (src,dst) per mode)
        // We'll just add all and let Dijkstra pick the best
        auto AddFastEdge = [&](std::size_t src, std::size_t dst, double time, EMode mode){
            DFastAdj[src].push_back({dst, time, mode});
        };

        // Add edges from ways
        for(std::size_t wi = 0; wi < DStreetMap->WayCount(); wi++){
            auto Way = DStreetMap->WayByIndex(wi);
            bool OneWay = Way->HasAttribute("oneway") && Way->GetAttribute("oneway") == "yes";

            // Determine speed limit for this way
            double SpeedLimit = DDefaultSpeedLimit;
            if(Way->HasAttribute("maxspeed")){
                auto SpeedStr = Way->GetAttribute("maxspeed");
                std::istringstream SS(SpeedStr);
                double Val;
                if(SS >> Val){
                    SpeedLimit = Val;
                }
            }

            for(std::size_t ni = 1; ni < Way->NodeCount(); ni++){
                auto SrcID = Way->GetNodeID(ni - 1);
                auto DstID = Way->GetNodeID(ni);
                auto SrcNode = DStreetMap->NodeByID(SrcID);
                auto DstNode = DStreetMap->NodeByID(DstID);
                if(!SrcNode || !DstNode) continue;

                auto SrcIt = DNodeIDToIndex.find(SrcID);
                auto DstIt = DNodeIDToIndex.find(DstID);
                if(SrcIt == DNodeIDToIndex.end() || DstIt == DNodeIDToIndex.end()) continue;

                std::size_t SrcIdx = SrcIt->second;
                std::size_t DstIdx = DstIt->second;

                double Dist = SGeographicUtils::HaversineDistanceInMiles(
                    SrcNode->Location(), DstNode->Location());

                // Shortest router
                DShortestRouter->AddEdge(SrcIdx, DstIdx, Dist, !OneWay);

                        // Walk is always bidirectional regardless of oneway
                double WalkTime = Dist / DWalkSpeed;
                AddFastEdge(SrcIdx, DstIdx, WalkTime, EMode::Walk);
                AddFastEdge(DstIdx, SrcIdx, WalkTime, EMode::Walk);

                // Bike follows oneway and is blocked on bicycle=no ways
                bool NoBike = Way->HasAttribute("bicycle") && Way->GetAttribute("bicycle") == "no";
                if(!NoBike){
                    double BikeTime = Dist / DBikeSpeed;
                    AddFastEdge(SrcIdx, DstIdx, BikeTime, EMode::Bike);
                    if(!OneWay){
                        AddFastEdge(DstIdx, SrcIdx, BikeTime, EMode::Bike);
                    }
                }
            }
        }

        // Add bus edges: one directed edge per consecutive stop pair on each route
        for(std::size_t ri = 0; ri < DBusSystem->RouteCount(); ri++){
            auto Route = DBusSystem->RouteByIndex(ri);
            for(std::size_t si = 1; si < Route->StopCount(); si++){
                auto PrevStopID = Route->GetStopID(si - 1);
                auto CurrStopID = Route->GetStopID(si);
                auto PrevStop = DBusSystem->StopByID(PrevStopID);
                auto CurrStop = DBusSystem->StopByID(CurrStopID);
                if(!PrevStop || !CurrStop) continue;

                auto PrevNodeID = PrevStop->NodeID();
                auto CurrNodeID = CurrStop->NodeID();
                auto PrevNode = DStreetMap->NodeByID(PrevNodeID);
                auto CurrNode = DStreetMap->NodeByID(CurrNodeID);
                if(!PrevNode || !CurrNode) continue;

                auto PrevIt = DNodeIDToIndex.find(PrevNodeID);
                auto CurrIt = DNodeIDToIndex.find(CurrNodeID);
                if(PrevIt == DNodeIDToIndex.end() || CurrIt == DNodeIDToIndex.end()) continue;

                double Dist = SGeographicUtils::HaversineDistanceInMiles(
                    PrevNode->Location(), CurrNode->Location());

                // Find the road speed between these two nodes
                double SpeedLimit = GetSpeedBetween(PrevNodeID, CurrNodeID);
                double StopTimeHrs = DBusStopTime / 3600.0;
                double BusTime = Dist / SpeedLimit + StopTimeHrs;

                AddFastEdge(PrevIt->second, CurrIt->second, BusTime, EMode::Bus);
            }
        }

        auto Deadline = std::chrono::steady_clock::now() +
            std::chrono::seconds(DPrecomputeTime);
        DShortestRouter->Precompute(Deadline);
    }

    std::size_t NodeCount() const noexcept{
        return DSortedNodes.size();
    }

    std::shared_ptr<CStreetMap::SNode> SortedNodeByIndex(std::size_t index) const noexcept{
        if(index < DSortedNodes.size()){
            return DSortedNodes[index];
        }
        return nullptr;
    }

    double FindShortestPath(TNodeID src, TNodeID dest, std::vector<TNodeID> &path){
        auto SrcIt = DNodeIDToIndex.find(src);
        auto DstIt = DNodeIDToIndex.find(dest);
        if(SrcIt == DNodeIDToIndex.end() || DstIt == DNodeIDToIndex.end()){
            path.clear();
            return CPathRouter::NoPathExists;
        }

        std::vector<CPathRouter::TVertexID> VtxPath;
        double Dist = DShortestRouter->FindShortestPath(SrcIt->second, DstIt->second, VtxPath);
        if(Dist == CPathRouter::NoPathExists){
            path.clear();
            return CPathRouter::NoPathExists;
        }

        path.clear();
        for(auto VtxID : VtxPath){
            path.push_back(std::any_cast<TNodeID>(DShortestRouter->GetVertexTag(VtxID)));
        }
        return Dist;
    }

    double FindFastestPath(TNodeID src, TNodeID dest, std::vector<TTripStep> &path){
        auto SrcIt = DNodeIDToIndex.find(src);
        auto DstIt = DNodeIDToIndex.find(dest);
        if(SrcIt == DNodeIDToIndex.end() || DstIt == DNodeIDToIndex.end()){
            path.clear();
            return CPathRouter::NoPathExists;
        }

        std::size_t N = DSortedNodes.size();
        std::size_t SrcIdx = SrcIt->second;
        std::size_t DstIdx = DstIt->second;

        std::vector<double> Dist(N, std::numeric_limits<double>::max());
        std::vector<std::size_t> Prev(N, std::numeric_limits<std::size_t>::max());
        std::vector<EMode> PrevMode(N, EMode::Walk);

        Dist[SrcIdx] = 0.0;

        using TPQEntry = std::pair<double, std::size_t>;
        std::priority_queue<TPQEntry, std::vector<TPQEntry>, std::greater<TPQEntry>> PQ;
        PQ.push({0.0, SrcIdx});

        while(!PQ.empty()){
            auto [CurDist, CurIdx] = PQ.top();
            PQ.pop();

            if(CurDist > Dist[CurIdx]) continue;
            if(CurIdx == DstIdx) break;

            for(auto &Edge : DFastAdj[CurIdx]){
                double NewDist = Dist[CurIdx] + Edge.DTime;
                if(NewDist < Dist[Edge.DDestIdx]){
                    Dist[Edge.DDestIdx] = NewDist;
                    Prev[Edge.DDestIdx] = CurIdx;
                    PrevMode[Edge.DDestIdx] = Edge.DMode;
                    PQ.push({NewDist, Edge.DDestIdx});
                }
            }
        }

        if(Dist[DstIdx] == std::numeric_limits<double>::max()){
            path.clear();
            return CPathRouter::NoPathExists;
        }

        // Reconstruct path
        std::vector<std::size_t> IdxPath;
        for(std::size_t At = DstIdx; At != std::numeric_limits<std::size_t>::max(); At = Prev[At]){
            IdxPath.push_back(At);
            if(At == SrcIdx) break;
        }
        std::reverse(IdxPath.begin(), IdxPath.end());

        path.clear();
        for(std::size_t i = 0; i < IdxPath.size(); i++){
            EMode Mode;
            if(i == 0){
                // First node: use mode of first edge, but if first edge is Bus, use Walk
                if(IdxPath.size() > 1){
                    EMode FirstEdgeMode = PrevMode[IdxPath[1]];
                    Mode = (FirstEdgeMode == EMode::Bus) ? EMode::Walk : FirstEdgeMode;
                }
                else{
                    Mode = EMode::Walk;
                }
            }
            else{
                Mode = PrevMode[IdxPath[i]];
            }
            path.push_back({Mode, DSortedNodes[IdxPath[i]]->ID()});
        }

        return Dist[DstIdx];
    }

    bool GetPathDescription(const std::vector<TTripStep> &path,
                            std::vector<std::string> &desc) const {
        if(path.empty()) return false;

        desc.clear();

        auto StartNodeID = path[0].second;
        auto StartNode = DStreetMap->NodeByID(StartNodeID);
        if(!StartNode) return false;
        desc.push_back("Start at " + SGeographicUtils::ConvertLLToDMS(StartNode->Location()));

        std::size_t i = 1;
        while(i < path.size()){
            auto [Mode, NodeID] = path[i];

            if(Mode == EMode::Bus){
                // Find the bus segment
                auto PrevNodeID = path[i-1].second;

                // Find route name
                std::string RouteName = "";
                auto PrevStop = DBusSystemIndexer->StopByNodeID(PrevNodeID);
                auto CurrStop = DBusSystemIndexer->StopByNodeID(NodeID);
                if(PrevStop && CurrStop){
                    std::unordered_set<std::shared_ptr<CBusSystem::SRoute>> Routes;
                    if(DBusSystemIndexer->RoutesByNodeIDs(PrevNodeID, NodeID, Routes) && !Routes.empty()){
                        // Pick alphabetically first route name
                        for(auto &R : Routes){
                            if(RouteName.empty() || R->Name() < RouteName){
                                RouteName = R->Name();
                            }
                        }
                    }
                }

                // Find end of bus segment
                std::size_t BusEnd = i;
                while(BusEnd < path.size() && path[BusEnd].first == EMode::Bus){
                    BusEnd++;
                }
                auto LastBusNodeID = path[BusEnd - 1].second;
                auto LastBusStop = DBusSystemIndexer->StopByNodeID(LastBusNodeID);

                std::string FromStopStr = PrevStop ? std::to_string(PrevStop->ID()) : std::to_string(PrevNodeID);
                std::string ToStopStr = LastBusStop ? std::to_string(LastBusStop->ID()) : std::to_string(LastBusNodeID);

                desc.push_back("Take Bus " + RouteName + " from stop " + FromStopStr + " to stop " + ToStopStr);
                i = BusEnd;
            }
            else{
                // Walk or Bike segment
                auto PrevNodeID = path[i-1].second;
                auto CurNodeID = NodeID;

                std::string WayName = GetWayNameBetween(PrevNodeID, CurNodeID);
                double SegDist = 0.0;

                // Accumulate consecutive steps of same mode and same way name
                auto PrevID = PrevNodeID;
                std::size_t j = i;
                while(j < path.size() && path[j].first == Mode){
                    auto CurID = path[j].second;
                    std::string CurWayName = GetWayNameBetween(PrevID, CurID);
                    if(CurWayName != WayName){
                        break;
                    }
                    auto PrevNode = DStreetMap->NodeByID(PrevID);
                    auto CurNode = DStreetMap->NodeByID(CurID);
                    if(PrevNode && CurNode){
                        SegDist += SGeographicUtils::HaversineDistanceInMiles(
                            PrevNode->Location(), CurNode->Location());
                    }
                    PrevID = CurID;
                    j++;
                }

                // Direction
                auto FirstNode = DStreetMap->NodeByID(path[i-1].second);
                auto LastNode = DStreetMap->NodeByID(PrevID);
                std::string Direction = "";
                if(FirstNode && LastNode){
                    if(WayName.empty()){
                        // For unnamed road: use bisector only if previous step was same mode and unnamed
                        bool UseBisector = false;
                        if(i >= 2 && path[i-2].first == Mode){
                            std::string PrevWay = GetWayNameBetween(path[i-2].second, path[i-1].second);
                            if(PrevWay.empty()){
                                UseBisector = true;
                            }
                        }
                        if(UseBisector){
                            auto BeforeNode = DStreetMap->NodeByID(path[i-2].second);
                            if(BeforeNode && !(BeforeNode->Location() == FirstNode->Location())){
                                Direction = SGeographicUtils::CalcualteExternalBisectorDirection(
                                    BeforeNode->Location(), FirstNode->Location(), LastNode->Location());
                            }
                            else{
                                double Bearing = SGeographicUtils::CalculateBearing(FirstNode->Location(), LastNode->Location());
                                Direction = SGeographicUtils::BearingToDirection(Bearing);
                            }
                        }
                        else{
                            double Bearing = SGeographicUtils::CalculateBearing(FirstNode->Location(), LastNode->Location());
                            Direction = SGeographicUtils::BearingToDirection(Bearing);
                        }
                    }
                    else{
                        double Bearing = SGeographicUtils::CalculateBearing(FirstNode->Location(), LastNode->Location());
                        Direction = SGeographicUtils::BearingToDirection(Bearing);
                    }
                }

                std::string ModeStr = (Mode == EMode::Walk) ? "Walk" : "Bike";
                std::ostringstream SS;
                SS << ModeStr << " " << Direction;
                if(!WayName.empty()){
                    SS << " along " << WayName;
                }
                else{
                    // Find next named way for "toward" hint
                    std::string NextWayName = "";
                    if(j < path.size()){
                        NextWayName = GetWayNameBetween(PrevID, path[j].second);
                    }
                    if(!NextWayName.empty()){
                        SS << " toward " << NextWayName;
                    }
                    else{
                        SS << " toward End";
                    }
                }
                SS << " for ";
                // Format distance with 2 significant figures
                std::ostringstream DistSS;
                DistSS << std::setprecision(2) << std::noshowpoint << SegDist;
                SS << DistSS.str() << " mi";
                desc.push_back(SS.str());
                i = j;
            }
        }

        auto EndNodeID = path.back().second;
        auto EndNode = DStreetMap->NodeByID(EndNodeID);
        if(!EndNode) return false;
        desc.push_back("End at " + SGeographicUtils::ConvertLLToDMS(EndNode->Location()));

        return true;
    }

    // Returns the "name" attribute of the way containing edge src→dst, or "" if unnamed
    std::string GetWayNameBetween(TNodeID src, TNodeID dst) const {
        for(std::size_t wi = 0; wi < DStreetMap->WayCount(); wi++){
            auto Way = DStreetMap->WayByIndex(wi);
            for(std::size_t ni = 1; ni < Way->NodeCount(); ni++){
                auto A = Way->GetNodeID(ni-1);
                auto B = Way->GetNodeID(ni);
                if((A == src && B == dst) || (A == dst && B == src)){
                    if(Way->HasAttribute("name")){
                        return Way->GetAttribute("name");
                    }
                    return "";
                }
            }
        }
        return "";
    }
};

CDijkstraTransportationPlanner::CDijkstraTransportationPlanner(std::shared_ptr<SConfiguration> config){
    DImplementation = std::make_unique<SImplementation>(config);
}

CDijkstraTransportationPlanner::~CDijkstraTransportationPlanner(){
}

std::size_t CDijkstraTransportationPlanner::NodeCount() const noexcept{
    return DImplementation->NodeCount();
}

std::shared_ptr<CStreetMap::SNode> CDijkstraTransportationPlanner::SortedNodeByIndex(std::size_t index) const noexcept{
    return DImplementation->SortedNodeByIndex(index);
}

double CDijkstraTransportationPlanner::FindShortestPath(TNodeID src, TNodeID dest, std::vector<TNodeID> &path){
    return DImplementation->FindShortestPath(src, dest, path);
}

double CDijkstraTransportationPlanner::FindFastestPath(TNodeID src, TNodeID dest, std::vector<TTripStep> &path){
    return DImplementation->FindFastestPath(src, dest, path);
}

bool CDijkstraTransportationPlanner::GetPathDescription(const std::vector<TTripStep> &path, std::vector<std::string> &desc) const{
    return DImplementation->GetPathDescription(path, desc);
}
