#include "CSVBusSystem.h"
#include <unordered_map>
#include <vector>
#include <string>

// Out-of-line definition required for ODR; value is set in the header
const CBusSystem::TStopID CBusSystem::InvalidStopID;

struct CCSVBusSystem::SImplementation{

    // Concrete stop: holds the stop ID and its corresponding street-map node ID
    struct SStop: public CBusSystem::SStop{
        TStopID DID;
        CStreetMap::TNodeID DNodeID;

        TStopID ID() const noexcept override{
            return DID;
        }

        CStreetMap::TNodeID NodeID() const noexcept override{
            return DNodeID;
        }
    };

    // Concrete route: holds the route name and the ordered list of stop IDs
    struct SRoute: public CBusSystem::SRoute{
        std::string DName;
        std::vector<TStopID> DStopIDs;

        std::string Name() const noexcept override{
            return DName;
        }

        std::size_t StopCount() const noexcept override{
            return DStopIDs.size();
        }

        // Returns InvalidStopID when index is out of range
        TStopID GetStopID(std::size_t index) const noexcept override{
            if (index >= DStopIDs.size()) return InvalidStopID;
            return DStopIDs[index];
        }
    };

    const std::string STOP_ID_HEADER    = "stop_id";
    const std::string NODE_ID_HEADER    = "node_id";
    const std::string ROUTE_HEADER      = "route";

    // Stops stored in insertion order for index-based access, and by ID for O(1) lookup
    std::vector< std::shared_ptr< SStop > > DStopsByIndex;
    std::unordered_map< TStopID, std::shared_ptr< SStop > > DStopsByID;

    // Routes stored in insertion order for index-based access, and by name for O(1) lookup
    std::vector< std::shared_ptr< SRoute > > DRoutesByIndex;
    std::unordered_map< std::string, std::shared_ptr< SRoute > > DRoutesByName;

    // Reads the header row to find column positions, then parses each stop row
    bool ReadStops(std::shared_ptr< CDSVReader > stopsrc){
        std::vector<std::string> TempRow;

        if(stopsrc->ReadRow(TempRow)){
            size_t StopColumn = static_cast<size_t>(-1);
            size_t NodeColumn = static_cast<size_t>(-1);
            for(size_t Index = 0; Index < TempRow.size(); Index++){
                if(TempRow[Index] == STOP_ID_HEADER){
                    StopColumn = Index;
                }
                else if(TempRow[Index] == NODE_ID_HEADER){
                    NodeColumn = Index;
                }
            }
            if(StopColumn == static_cast<size_t>(-1) || NodeColumn == static_cast<size_t>(-1)){
                return false;
            }
            while(stopsrc->ReadRow(TempRow)){
                if(StopColumn >= TempRow.size() || NodeColumn >= TempRow.size())
                    continue;
                const std::string &stopStr = TempRow[StopColumn];
                const std::string &nodeStr = TempRow[NodeColumn];
                if(stopStr.empty() || nodeStr.empty())
                    continue;
                if(stopStr == STOP_ID_HEADER || nodeStr == NODE_ID_HEADER)
                    continue; /* skip header if repeated */
                try {
                    TStopID StopID = std::stoull(stopStr);
                    CStreetMap::TNodeID NodeID = std::stoull(nodeStr);
                    auto NewStop = std::make_shared< SStop >();
                    NewStop->DID = StopID;
                    NewStop->DNodeID = NodeID;
                    DStopsByIndex.push_back(NewStop);
                    DStopsByID[StopID] = NewStop;
                } catch (...) {
                    /* skip malformed row */
                }
            }
            return true;
        }
        return false;
    }

    // Reads the header row to find column positions, then parses each route row.
    // Consecutive rows with the same route name are grouped into a single SRoute;
    // a new route object is created only when the name changes.
    bool ReadRoutes(std::shared_ptr< CDSVReader > routesrc){
        std::vector<std::string> TempRow;
        if(!routesrc->ReadRow(TempRow)) return false;
        size_t RouteColumn = static_cast<size_t>(-1);
        size_t StopColumn = static_cast<size_t>(-1);
        for(size_t i = 0; i < TempRow.size(); i++){
            if(TempRow[i] == ROUTE_HEADER) RouteColumn = i;
            else if(TempRow[i] == STOP_ID_HEADER) StopColumn = i;
        }
        if(RouteColumn == static_cast<size_t>(-1) || StopColumn == static_cast<size_t>(-1))
            return false;
        std::string currentName;
        std::shared_ptr<SRoute> currentRoute;
        while(routesrc->ReadRow(TempRow)){
            if(RouteColumn >= TempRow.size() || StopColumn >= TempRow.size())
                continue;
            std::string name = TempRow[RouteColumn];
            const std::string& stopStr = TempRow[StopColumn];
            if(stopStr.empty())
                continue;
            if(name == ROUTE_HEADER && stopStr == STOP_ID_HEADER)
                continue; /* skip header if repeated */
            try {
                TStopID stopId = std::stoull(stopStr);
                if(name != currentName){
                    currentName = name;
                    currentRoute = std::make_shared<SRoute>();
                    currentRoute->DName = name;
                    DRoutesByIndex.push_back(currentRoute);
                    DRoutesByName[name] = currentRoute;
                }
                currentRoute->DStopIDs.push_back(stopId);
            } catch (...) {}
        }
        return true;
    }

    SImplementation(std::shared_ptr< CDSVReader > stopsrc, std::shared_ptr< CDSVReader > routesrc){
        ReadStops(stopsrc);
        ReadRoutes(routesrc);
    }

    std::size_t StopCount() const noexcept{
        return DStopsByIndex.size();
    }

    std::size_t RouteCount() const noexcept{
        return DRoutesByIndex.size();
    }

    std::shared_ptr<SStop> StopByIndex(std::size_t index) const noexcept{
        if(index >= DStopsByIndex.size()) return nullptr;
        return DStopsByIndex[index];
    }

    std::shared_ptr<SStop> StopByID(TStopID id) const noexcept{
        auto it = DStopsByID.find(id);
        if(it == DStopsByID.end()) return nullptr;
        return it->second;
    }

    std::shared_ptr<SRoute> RouteByIndex(std::size_t index) const noexcept{
        if(index >= DRoutesByIndex.size()) return nullptr;
        return DRoutesByIndex[index];
    }

    std::shared_ptr<SRoute> RouteByName(const std::string &name) const noexcept{
        auto it = DRoutesByName.find(name);
        if(it == DRoutesByName.end()) return nullptr;
        return it->second;
    }


};
    
CCSVBusSystem::CCSVBusSystem(std::shared_ptr< CDSVReader > stopsrc, std::shared_ptr< CDSVReader > routesrc){
    DImplementation = std::make_unique<SImplementation>(stopsrc,routesrc);
}

CCSVBusSystem::~CCSVBusSystem(){

}

std::size_t CCSVBusSystem::StopCount() const noexcept{
    return DImplementation->StopCount();
}

std::size_t CCSVBusSystem::RouteCount() const noexcept{
    return DImplementation->RouteCount();
}

std::shared_ptr<CBusSystem::SStop> CCSVBusSystem::StopByIndex(std::size_t index) const noexcept{
    return DImplementation->StopByIndex(index);
}

std::shared_ptr<CBusSystem::SStop> CCSVBusSystem::StopByID(TStopID id) const noexcept{
    return DImplementation->StopByID(id);
}

std::shared_ptr<CBusSystem::SRoute> CCSVBusSystem::RouteByIndex(std::size_t index) const noexcept{
    return DImplementation->RouteByIndex(index);
}

std::shared_ptr<CBusSystem::SRoute> CCSVBusSystem::RouteByName(const std::string &name) const noexcept{
    return DImplementation->RouteByName(name);
}



