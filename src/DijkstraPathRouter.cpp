#include "DijkstraPathRouter.h"
#include <queue>
#include <limits>
#include <algorithm>
#include <unordered_map>

struct CDijkstraPathRouter::SImplementation{
    struct SVertex;
    // Edge: (weight, destination vertex)
    using TEdge = std::pair<double,std::shared_ptr<SVertex>>;
    struct SVertex{
        std::vector<TEdge> DEdges;
        std::any DTag;
    };
    std::vector<std::shared_ptr<SVertex>> DVertices;
    // Reverse map from raw pointer to vertex ID for O(1) lookup during path relaxation
    std::unordered_map<SVertex*, TVertexID> DVertexToID;

    SImplementation(){}
    ~SImplementation(){}

    std::size_t VertexCount() const noexcept{
        return DVertices.size();
    }

    TVertexID AddVertex(std::any tag) noexcept{
        auto NewVertex = std::make_shared<SVertex>();
        NewVertex->DTag = tag;
        TVertexID NewID = DVertices.size();
        DVertexToID[NewVertex.get()] = NewID;
        DVertices.push_back(NewVertex);
        return NewID;
    }

    std::any GetVertexTag(TVertexID id) const noexcept{
        if(id < DVertices.size()){
            return DVertices[id]->DTag;
        }
        return std::any();
    }

    bool AddEdge(TVertexID src, TVertexID dest, double weight, bool bidir = false) noexcept{
        // Reject invalid vertex IDs or negative weights
        if(src < DVertices.size() && dest < DVertices.size() && weight >= 0.0){
            DVertices[src]->DEdges.push_back(std::make_pair(weight,DVertices[dest]));
            if(bidir){
                DVertices[dest]->DEdges.push_back(std::make_pair(weight,DVertices[src]));
            }
            return true;
        }
        return false;
    }

    bool Precompute(std::chrono::steady_clock::time_point deadline) noexcept{
        return true;
    }

    double FindShortestPath(TVertexID src, TVertexID dest, std::vector<TVertexID> &path) noexcept{
        if(src >= DVertices.size() || dest >= DVertices.size()){
            return CPathRouter::NoPathExists;
        }
        std::vector<double> Dist(DVertices.size(), std::numeric_limits<double>::max());
        std::vector<TVertexID> Prev(DVertices.size(), CPathRouter::InvalidVertexID);
        Dist[src] = 0.0;

        // Min-heap: (distance, vertex ID)
        using TPair = std::pair<double, TVertexID>;
        std::priority_queue<TPair, std::vector<TPair>, std::greater<TPair>> PQ;
        PQ.push({0.0, src});

        while(!PQ.empty()){
            auto [D, U] = PQ.top();
            PQ.pop();
            if(D > Dist[U]) continue; // stale entry
            if(U == dest) break;
            for(auto &[W, VPtr] : DVertices[U]->DEdges){
                auto It = DVertexToID.find(VPtr.get());
                if(It == DVertexToID.end()) continue;
                TVertexID V = It->second;
                double NewDist = Dist[U] + W;
                if(NewDist < Dist[V]){
                    Dist[V] = NewDist;
                    Prev[V] = U;
                    PQ.push({NewDist, V});
                }
            }
        }

        if(Dist[dest] == std::numeric_limits<double>::max()){
            return CPathRouter::NoPathExists;
        }

        // Reconstruct path by walking predecessor chain from dest back to src
        path.clear();
        for(TVertexID At = dest; At != CPathRouter::InvalidVertexID; At = Prev[At]){
            path.push_back(At);
        }
        std::reverse(path.begin(), path.end());
        return Dist[dest];
    }
};

CDijkstraPathRouter::CDijkstraPathRouter(){
    DImplementation = std::make_unique<SImplementation>();
}

CDijkstraPathRouter::~CDijkstraPathRouter(){

}

std::size_t CDijkstraPathRouter::VertexCount() const noexcept{
    return DImplementation->VertexCount();
}

CPathRouter::TVertexID CDijkstraPathRouter::AddVertex(std::any tag) noexcept{
    return DImplementation->AddVertex(tag);
}

std::any CDijkstraPathRouter::GetVertexTag(TVertexID id) const noexcept{
    return DImplementation->GetVertexTag(id);
}

bool CDijkstraPathRouter::AddEdge(TVertexID src, TVertexID dest, double weight, bool bidir) noexcept{
    return DImplementation->AddEdge(src,dest,weight,bidir);
}

bool CDijkstraPathRouter::Precompute(std::chrono::steady_clock::time_point deadline) noexcept{
    return DImplementation->Precompute(deadline);
}

double CDijkstraPathRouter::FindShortestPath(TVertexID src, TVertexID dest, std::vector<TVertexID> &path) noexcept{
    return DImplementation->FindShortestPath(src,dest,path);
}
