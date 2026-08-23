// XMLWriter.cpp — XML stream writer
// Writes SXMLEntity events to a CDataSink; escapes special chars and can flush unclosed elements.

#include "XMLWriter.h"
#include <stack>
#include <vector>

// Escape XML special characters in attribute values and character data (& < > " ').
static std::string EscapeXML(const std::string &str) {
    std::string result;
    result.reserve(str.size());
    for (char c : str) {
        switch (c) {
            case '&':  result += "&amp;";  break;
            case '<':  result += "&lt;";   break;
            case '>':  result += "&gt;";   break;
            case '"':  result += "&quot;"; break;
            case '\'': result += "&apos;"; break;
            default:   result += c;        break;
        }
    }
    return result;
}

// Helper: write a string to the sink as a char buffer.
static void WriteString(CDataSink &sink, const std::string &s) {
    std::vector<char> buf(s.begin(), s.end());
    sink.Write(buf);
}

// Pimpl: holds sink and stack of open element names for Flush().
struct CXMLWriter::SImplementation {
    std::shared_ptr<CDataSink> DSink;
    std::stack<std::string> DOpenElements;  // Track open tags for Flush() to close

    SImplementation(std::shared_ptr<CDataSink> sink) : DSink(sink) {}

    // Emit one entity: StartElement/EndElement/CharData/CompleteElement; StartElement pushes to DOpenElements.
    bool WriteEntity(const SXMLEntity &entity) {
        switch (entity.DType) {
            case SXMLEntity::EType::StartElement: {
                DOpenElements.push(entity.DNameData);
                std::string out = "<" + entity.DNameData;
                for (const auto &attr : entity.DAttributes) {
                    out += " " + attr.first + "=\"" + EscapeXML(attr.second) + "\"";
                }
                out += ">";
                WriteString(*DSink, out);
                return true;
            }
            case SXMLEntity::EType::EndElement: {
                if (!DOpenElements.empty() && DOpenElements.top() == entity.DNameData) {
                    DOpenElements.pop();
                }
                WriteString(*DSink, "</" + entity.DNameData + ">");
                return true;
            }
            case SXMLEntity::EType::CharData: {
                WriteString(*DSink, EscapeXML(entity.DNameData));
                return true;
            }
            case SXMLEntity::EType::CompleteElement: {
                std::string out = "<" + entity.DNameData;
                for (const auto &attr : entity.DAttributes) {
                    out += " " + attr.first + "=\"" + EscapeXML(attr.second) + "\"";
                }
                out += "/>";
                WriteString(*DSink, out);
                return true;
            }
        }
        return false;
    }

    // Close any elements still on the stack by writing </name> for each.
    bool Flush() {
        while (!DOpenElements.empty()) {
            std::string name = DOpenElements.top();
            DOpenElements.pop();
            WriteString(*DSink, "</" + name + ">");
        }
        return true;
    }
};

// --- Public API ---

CXMLWriter::CXMLWriter(std::shared_ptr<CDataSink> sink)
    : DImplementation(std::make_unique<SImplementation>(sink)) {}

CXMLWriter::~CXMLWriter() {
    DImplementation->Flush();
}

bool CXMLWriter::Flush() {
    return DImplementation->Flush();
}

bool CXMLWriter::WriteEntity(const SXMLEntity &entity) {
    return DImplementation->WriteEntity(entity);
}
