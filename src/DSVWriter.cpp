// DSVWriter.cpp — Delimiter-Separated Values writer
// Writes rows to a CDataSink with configurable delimiter (e.g. ',' or '\t').

#include "DSVWriter.h"

// Pimpl: holds data sink, delimiter, and quote-all flag; WriteRow emits one row.
struct CDSVWriter::SImplementation{
    std::shared_ptr< CDataSink > DSink;
    char DDelimiter;
    bool DQuoteAll;  // If true, fields could be quoted when needed (currently unused in write path)

    SImplementation(std::shared_ptr< CDataSink > sink, char delimiter, bool quoteall){
        DSink = sink;
        DDelimiter = delimiter;
        DQuoteAll = quoteall;
    }

    ~SImplementation(){

    }

    // Write one row: each column written as-is, separated by DDelimiter; no quoting applied.
    bool WriteRow(const std::vector<std::string> &row){
        bool First = true;
        for(auto &Column : row){
            std::vector<char> Buffer(Column.begin(),Column.end());
            if(!First){
                DSink->Put(DDelimiter);
            }
            First = false;
            DSink->Write(Buffer);
        }
        return true;
    }

};

// --- Public API ---

CDSVWriter::CDSVWriter(std::shared_ptr< CDataSink > sink, char delimiter, bool quoteall){
    DImplementation = std::make_unique<SImplementation>(sink,delimiter,quoteall);
}

CDSVWriter::~CDSVWriter(){
    DImplementation.reset();
}

bool CDSVWriter::WriteRow(const std::vector<std::string> &row){
    return DImplementation->WriteRow(row);
}
