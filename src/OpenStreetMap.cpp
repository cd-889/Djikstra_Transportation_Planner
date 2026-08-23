#include "OpenStreetMap.h"
#include <unordered_map>

struct COpenStreetMap::SImplementation{
    const std::string DOSMTag = "osm";
    const std::string DNodeTag = "node";
    const std::string DNodeIDAttr = "id";
    const std::string DNodeLatAttr = "lat";
    const std::string DNodeLonAttr = "lon";

    struct SNode: public CStreetMap::SNode{
        TNodeID DID;
        SLocation DLocation;
        ~SNode(){};
        
        std::vector<std::pair<std::string,std::string>> DAttributes; // order of attributes for indexes 
        std::unordered_map<std::string, std::string> DAttributeMap; // for quick lookup 
        // Street Map Node member functions
        // Returns the id of the SNode
        TNodeID ID() const noexcept override
        {
            return DID;   
        }

        SLocation Location() const noexcept override{
            return DLocation;
        }

        // Returns the number of attributes attached to the SNode
        std::size_t AttributeCount() const noexcept override{
            return DAttributes.size(); 
        }

        // Returns the key of the attribute at index, returns empty string if index
        // is greater than or equal to AttributeCount()
        std::string GetAttributeKey(std::size_t index) const noexcept override{
            if (index >=DAttributes.size())
            {
                return ""; 
            }
            else
            {
                return DAttributes[index].first; 
            }
        }
        // Returns if the attribute is attached to the SNode
        bool HasAttribute(const std::string &key) const noexcept override{
            if (DAttributeMap.find(key) != DAttributeMap.end())
            {
                return 1; 
            }
            else
            {
                return 0; 
            }
        }
        // Returns the value of the attribute specified by key, returns empty string
        // if key hasn't been attached to SNode
        std::string GetAttribute(const std::string &key) const noexcept override{
            if (HasAttribute(key))
            {
                return DAttributeMap.at(key); 
            }
            else
            {
                return ""; 
            }
        }
    };

    struct SWay: public CStreetMap::SWay{
        ~SWay(){};

        TWayID DID{}; 
        std::vector<TNodeID> DNodeIDs; // this will the ordered node references of way 
        std::vector <std::pair <std::string, std::string>> DAttributes; // this is to keep order of the indexes 
        std::unordered_map<std::string, std::string> DAttributeMap; // this is another quick lookup 
        
        // Returns the id of the SWay
        TWayID ID() const noexcept override{ 
            return DID; 
        }

        // Returns the number of nodes in the way
        std::size_t NodeCount() const noexcept override{
            return DNodeIDs.size(); 
        }
        
        // Returns the node id of the node at index, returns InvalidNodeID if index
// is greater than or equal to NodeCount()
        TNodeID GetNodeID(std::size_t index) const noexcept override{
            if (index >=DNodeIDs.size())
            {
                return TNodeID{}; 
            }
            else{
                return DNodeIDs[index]; 
            }
        }

        // Returns the number of attributes attached to the SWay
        std::size_t AttributeCount() const noexcept override{
            return DAttributes.size(); 
        }

        // Returns the key of the attribute at index, returns empty string if index
        // is greater than or equal to AttributeCount()
        std::string GetAttributeKey(std::size_t index) const noexcept override{
            if (index >= AttributeCount())
            {
                return ""; 
            }
            else{
                return DAttributes[index].first; // to get the key at the specific index since it's a vector of pairs 
            }
        }

        bool HasAttribute(const std::string &key) const noexcept override{
            if (DAttributeMap.find(key) != DAttributeMap.end())
            {
                return 1; 
            }
            else
            {
                return 0; 
            }
        }

        std::string GetAttribute(const std::string &key) const noexcept override{
            if (HasAttribute(key))
            {
                return DAttributeMap.at(key); 
            }
            else
            {
                return ""; 
            }
        }
    };
    
    std::vector<std::shared_ptr<SNode>> DNodesByIndex;
    std::unordered_map<TNodeID,std::shared_ptr<SNode>> DNodesByID;
    std::vector<std::shared_ptr<SWay>> DWaysByIndex; // ways 
    std::unordered_map<TWayID, std::shared_ptr<SWay>> DWaysByID; // ways by id 

    bool ParseNodes(std::shared_ptr<CXMLReader> src, SXMLEntity &nextentity){
        SXMLEntity TempEntity;

        while(src->ReadEntity(TempEntity)){
            if(TempEntity.DType == SXMLEntity::EType::StartElement && TempEntity.DNameData == DNodeTag){
                auto NodeID = std::stoull(TempEntity.AttributeValue(DNodeIDAttr));
                auto NodeLat = std::stod(TempEntity.AttributeValue(DNodeLatAttr));
                auto NodeLon = std::stod(TempEntity.AttributeValue(DNodeLonAttr));
                auto NewNode = std::make_shared<SNode>();
                NewNode->DID = NodeID;
                NewNode->DLocation = SLocation(NodeLat,NodeLon);
                DNodesByIndex.push_back(NewNode);
                DNodesByID[NodeID] = NewNode;
                // drain children of this node until </node>
                while(src->ReadEntity(TempEntity)){
                    if(TempEntity.DType == SXMLEntity::EType::StartElement && TempEntity.DNameData == "tag"){
                        auto k = TempEntity.AttributeValue("k");
                        auto v = TempEntity.AttributeValue("v");
                        if(!k.empty()){
                            NewNode->DAttributes.push_back({k, v});
                            NewNode->DAttributeMap[k] = v;
                        }
                    }
                    else if(TempEntity.DType == SXMLEntity::EType::EndElement && TempEntity.DNameData == DNodeTag){
                        break; // done with this node's children, continue outer loop
                    }
                    // skip CharData (whitespace) and EndElement "tag"
                }
                // outer loop continues to next node — do NOT break here
            } else if(TempEntity.DType == SXMLEntity::EType::CharData){
                // skip whitespace / text between elements
            } else {
                // first non-whitespace, non-node entity (e.g. <way>) — hand off to ParseWays
                nextentity = TempEntity;
                return true;
            }
        }
        return true;
    }


    bool ParseWays(std::shared_ptr<CXMLReader> src, SXMLEntity &firstentity){
        auto parse_way = [&](const SXMLEntity &waystart)
        {
            auto New_Way = std::make_shared<SWay>(); 
            std::string idstr = waystart.AttributeValue("id"); 
            if (!idstr.empty()){
                New_Way -> DID = static_cast<TWayID>(std::stoull(idstr)); 
            }

            SXMLEntity inner;
            while (src->ReadEntity(inner))
            {
                if (inner.DType == SXMLEntity::EType::StartElement && inner.DNameData == "nd")
                {
                    std::string ref = inner.AttributeValue("ref"); 
                    if (!ref.empty())
                    {
                        New_Way -> DNodeIDs.push_back(static_cast<TNodeID>(std::stoull(ref))); 
                    }
                }
                else if(inner.DType == SXMLEntity::EType::StartElement && inner.DNameData == "tag")
                {
                    std::string key = inner.AttributeValue("k"); 
                    std::string val = inner.AttributeValue("v"); 
                    if (!key.empty())
                    {
                        New_Way -> DAttributes.push_back({key,val}); 
                        New_Way -> DAttributeMap[key] = val; 
                    }
                }
                else if (inner.DType == SXMLEntity::EType::EndElement && inner.DNameData == "way")
                {
                    break; 
                }


            }
            DWaysByIndex.push_back(New_Way); 
            DWaysByID[New_Way -> DID] = New_Way; 

        }; 
    if (firstentity.DType == SXMLEntity::EType::StartElement && firstentity.DNameData == "way")
    {
        parse_way(firstentity); 
    }
    else if(firstentity.DType == SXMLEntity::EType::EndElement && firstentity.DNameData == DOSMTag)
    {
        return true;
    }
    // else: firstentity was something else—just proceed.

    // ---- Case B: continue scanning for more <way> elements ----
    SXMLEntity Temp;
    while(src->ReadEntity(Temp)){
        if(Temp.DType == SXMLEntity::EType::StartElement && Temp.DNameData == "way"){
            parse_way(Temp);
        }
        else if(Temp.DType == SXMLEntity::EType::EndElement && Temp.DNameData == DOSMTag){
            break;
        }
        // skip CharData (whitespace) and any other elements
    }
    return true;        
}

    bool ParseOpenStreetMap(std::shared_ptr<CXMLReader> src){
        SXMLEntity TempEntity;
        
        if(src->ReadEntity(TempEntity)){
            if(TempEntity.DType == SXMLEntity::EType::StartElement && TempEntity.DNameData == DOSMTag){
                if(ParseNodes(src,TempEntity)){
                    if(ParseWays(src,TempEntity)){
                        return true;
                    }
                }
            }
        }
        return false;
    }

    SImplementation(std::shared_ptr<CXMLReader> src){
        if(ParseOpenStreetMap(src)){

        }
    }

    std::size_t NodeCount() const noexcept{
        return DNodesByIndex.size();
    }

    std::size_t WayCount() const noexcept{
        return DWaysByIndex.size();
    }
    // returns SNode associated with index, returns nullptr if index is
// larger than or equal to NodeCount()
    std::shared_ptr<CStreetMap::SNode> NodeByIndex(std::size_t index) const noexcept{
        if (index >= NodeCount())
        {
            return nullptr; 
        }
        else
        {
            return DNodesByIndex[index]; 
        }
        ;
    }
    
    // returns SNode with id of id, returns nullptr if doesn't exist 
    std::shared_ptr<CStreetMap::SNode> NodeByID(TNodeID id) const noexcept{
        if (DNodesByID.find(id) != DNodesByID.end())
        {
            return DNodesByID.at(id); 
        }
        return nullptr;
    }
    // Returns the SWay associated with index, returns nullptr if index is
// larger than or equal to WayCount()
    std::shared_ptr<CStreetMap::SWay> WayByIndex(std::size_t index) const noexcept{
        if (index >= WayCount())
        {
            return nullptr; 
        }
        else{
            return DWaysByIndex[index]; 
        }
    }
    // Returns the SWay with the id of id, returns nullptr if doesn't exist
    std::shared_ptr<CStreetMap::SWay> WayByID(TWayID id) const noexcept{
        if (DWaysByID.find(id) == DWaysByID.end())
        {
            return nullptr; 
        }
        else
        {
            return DWaysByID.at(id); 
        }

    }
};

COpenStreetMap::COpenStreetMap(std::shared_ptr<CXMLReader> src){
    DImplementation = std::make_unique<SImplementation>(src);
}

COpenStreetMap::~COpenStreetMap(){

}

std::size_t COpenStreetMap::NodeCount() const noexcept {
    return DImplementation->NodeCount();
}
std::size_t COpenStreetMap::WayCount() const noexcept{
    return DImplementation->WayCount();
}

std::shared_ptr<CStreetMap::SNode> COpenStreetMap::NodeByIndex(std::size_t index) const noexcept{
    return DImplementation->NodeByIndex(index);
}

std::shared_ptr<CStreetMap::SNode> COpenStreetMap::NodeByID(TNodeID id) const noexcept{
    return DImplementation->NodeByID(id);
}
std::shared_ptr<CStreetMap::SWay> COpenStreetMap::WayByIndex(std::size_t index) const noexcept{
    return DImplementation->WayByIndex(index);
}

std::shared_ptr<CStreetMap::SWay> COpenStreetMap::WayByID(TWayID id) const noexcept{
    return DImplementation->WayByID(id);
}
