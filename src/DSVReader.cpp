// DSVReader.cpp — Delimiter-Separated Values reader
// Reads rows from a CDataSource, supporting quoted fields and escaped quotes.

#include "DSVReader.h"

// Pimpl: holds data source and delimiter; ParseValue/ReadRow implement row parsing.
struct CDSVReader::SImplementation{
    std::shared_ptr<CDataSource> DSource;
    char DDelimiter;

    SImplementation(std::shared_ptr< CDataSource > src, char delimiter){
        DSource = src;
        DDelimiter = delimiter;
    }

    // Parse one field; do not consume the terminating delimiter or newline.
    // Handles quoted strings: "" inside quotes becomes a literal quote.
    bool ParseValue(std::string &val){
        bool InQuotes = false;
        val.clear();
        while(true){
            if(DSource->End())
                return true;
            char NextChar;
            if(!DSource->Peek(NextChar))
                return true;
            if(!InQuotes && (NextChar == DDelimiter || NextChar == '\n'))
                return true;
            if(NextChar == '"'){
                if(!DSource->Get(NextChar))
                    return true;
                if(InQuotes){
                    if(!DSource->Peek(NextChar))
                        return true;
                    if(NextChar == '"'){
                        DSource->Get(NextChar);
                        val += '"';
                    } else
                        InQuotes = false;
                } else
                    InQuotes = true;
            } else {
                if(!DSource->Get(NextChar))
                    return true;
                val += NextChar;
            }
        }
    }

    // Report whether the underlying source has no more data.
    bool End() const{
        return DSource->End();
    }

    // Read one row: parse fields until newline or source end; return true if at least one field read.
    bool ReadRow(std::vector<std::string> &row){
        row.clear();
        if(DSource->End())
            return false;
        while(true){
            std::string NextValue;
            if(!ParseValue(NextValue))
                break;
            row.push_back(NextValue);
            if(DSource->End())
                return true;
            char c;
            if(!DSource->Peek(c))
                return true;
            if(c == '\n'){
                DSource->Get(c);
                return true;
            }
            if(c == DDelimiter)
                DSource->Get(c);
        }
        return !row.empty();
    }

};

// --- Public API ---

CDSVReader::CDSVReader(std::shared_ptr< CDataSource > src, char delimiter){
    DImplementation = std::make_unique<SImplementation>(src,delimiter);
}

CDSVReader::~CDSVReader(){
    DImplementation.reset();
}

bool CDSVReader::End() const{
    return DImplementation->End();
}

bool CDSVReader::ReadRow(std::vector<std::string> &row){
    return DImplementation->ReadRow(row);
}
