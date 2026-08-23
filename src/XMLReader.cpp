// XMLReader.cpp — XML stream reader using Expat
// Parses XML from a CDataSource and exposes events as SXMLEntity (StartElement, EndElement, CharData).

#include "XMLReader.h"
#include <expat.h>
#include <queue>

// Pimpl: Expat callbacks push entities into DQueue; ReadEntity() drains the queue and refills by parsing more input.
// Entity model: <tag attr="val">CharData</tag>

struct CXMLReader::SImplementation{
    std::shared_ptr< CDataSource > DSource;
    XML_Parser DParser;
    std::queue<SXMLEntity> DQueue;

    // Expat callback: push a StartElement entity with tag name and attributes.
    static void StartElementHandler(void *userData, const XML_Char *name, const XML_Char **atts){
        SImplementation *This = (SImplementation *)userData;
        SXMLEntity NewEntity;
        
        NewEntity.DType = SXMLEntity::EType::StartElement;
        NewEntity.DNameData = name;
        for(int i = 0; atts[i]; i += 2){
            NewEntity.DAttributes.push_back({atts[i],atts[i+1]});
        }
        This->DQueue.push(NewEntity);
    }

    // Expat callback: push an EndElement entity with tag name.
    static void EndElementHandler(void *userData, const XML_Char *name){
        SImplementation *This = (SImplementation *)userData;
        SXMLEntity NewEntity;
        
        NewEntity.DType = SXMLEntity::EType::EndElement;
        NewEntity.DNameData = name;
        This->DQueue.push(NewEntity);
    }

    // Expat callback: push a CharData entity with the text segment.
    static void CharacterDataHandler(void *userData, const XML_Char *s, int len){
        SImplementation *This = (SImplementation *)userData;
        SXMLEntity NewEntity;
        
        NewEntity.DType = SXMLEntity::EType::CharData;
        NewEntity.DNameData = std::string(s,len);
        This->DQueue.push(NewEntity);
    }

    // Create parser and register Expat callbacks for start/end elements and character data.
    SImplementation(std::shared_ptr< CDataSource > src){
        DSource = src;
        DParser = XML_ParserCreate(nullptr);
        XML_SetUserData(DParser,(void *)this);
        XML_SetStartElementHandler(DParser,StartElementHandler);
        XML_SetEndElementHandler(DParser,EndElementHandler);
        XML_SetCharacterDataHandler(DParser,CharacterDataHandler);

    }

    ~SImplementation(){
        XML_ParserFree(DParser);
    }

    // End() always true here; real EOF is when ReadEntity returns false after draining the queue.
    bool End() const{
        return true;
    }

    // Get next entity; if queue is empty, read more from source and parse. skipcdata is currently unused.
    bool ReadEntity(SXMLEntity &entity, bool skipcdata){

        if(DQueue.empty()){
            std::vector<char> Buffer(512);
            DSource->Read(Buffer,Buffer.size());
            XML_Parse(DParser,Buffer.data(),Buffer.size(),DSource->End());
        }
        if(!DQueue.empty()){
            entity = DQueue.front();
            DQueue.pop();
            return true;
        }

        return false;
    }

};

// --- Public API ---

CXMLReader::CXMLReader(std::shared_ptr< CDataSource > src){
    DImplementation = std::make_unique<SImplementation>(src);
}

CXMLReader::~CXMLReader(){
    DImplementation.reset();
}

bool CXMLReader::End() const{
    return DImplementation->End();
}

bool CXMLReader::ReadEntity(SXMLEntity &entity, bool skipcdata){
    return DImplementation->ReadEntity(entity,skipcdata);
}
