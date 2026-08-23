#include "TransportationPlannerCommandLine.h"
#include "DSVWriter.h"
#include "GeographicUtils.h"
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <cmath>

struct CTransportationPlannerCommandLine::SImplementation{
    std::shared_ptr<CDataSource> DCmdSrc;
    std::shared_ptr<CDataSink> DOutSink;
    std::shared_ptr<CDataSink> DErrSink;
    std::shared_ptr<CDataFactory> DResults;
    std::shared_ptr<CTransportationPlanner> DPlanner;

    // State retained between commands so save/print can reference the last path
    bool DHasPath = false;
    double DLastTime = 0.0;
    double DLastDist = 0.0;
    CStreetMap::TNodeID DLastSrc = 0;
    CStreetMap::TNodeID DLastDest = 0;
    bool DLastWasFastest = false;
    std::vector<CTransportationPlanner::TNodeID> DLastShortestPath;
    std::vector<CTransportationPlanner::TTripStep> DLastFastestPath;

    SImplementation(std::shared_ptr<CDataSource> cmdsrc,
                    std::shared_ptr<CDataSink> outsink,
                    std::shared_ptr<CDataSink> errsink,
                    std::shared_ptr<CDataFactory> results,
                    std::shared_ptr<CTransportationPlanner> planner)
        : DCmdSrc(cmdsrc), DOutSink(outsink), DErrSink(errsink),
          DResults(results), DPlanner(planner) {}

    // Writes every character of str to the given sink
    void WriteToSink(std::shared_ptr<CDataSink> sink, const std::string &str){
        for(std::size_t i = 0; i < str.length(); i++){
            sink->Put(str[i]);
        }
    }

    // Reads one line from DCmdSrc, stopping at '\n' or end-of-source
    std::string ReadLine(){
        std::string Line;
        char ch;
        while(!DCmdSrc->End()){
            DCmdSrc->Get(ch);
            if(ch == '\n') break;
            Line += ch;
        }
        return Line;
    }

    // Splits a line on whitespace and returns the tokens
    std::vector<std::string> SplitTokens(const std::string &line){
        std::istringstream Stream(line);
        std::string Token;
        std::vector<std::string> Tokens;
        while(Stream >> Token){
            Tokens.push_back(Token);
        }
        return Tokens;
    }

    // Converts a duration in hours to a human-readable string, e.g. "1 hr 22 min 30 sec".
    // Only non-zero components are included; returns "0 min" for zero duration.
    std::string FormatTime(double hours){
        std::string Result;
        int TotalSeconds = static_cast<int>(std::round(hours * 3600));
        int Hours   = TotalSeconds / 3600;
        int Minutes = (TotalSeconds % 3600) / 60;
        int Seconds = TotalSeconds % 60;

        if(Hours   > 0) Result += std::to_string(Hours)   + " hr ";
        if(Minutes > 0) Result += std::to_string(Minutes) + " min ";
        if(Seconds > 0) Result += std::to_string(Seconds) + " sec";

        if(Hours == 0 && Minutes == 0 && Seconds == 0) return "0 min";

        // Trim trailing space if present
        if(!Result.empty() && Result.back() == ' ') Result.pop_back();
        return Result;
    }

    // Returns true if every character in s is a digit (used for parameter validation)
    bool IsUnsignedInt(const std::string &s){
        for(auto ch : s){
            if(!std::isdigit(ch)) return false;
        }
        return !s.empty();
    }

    bool ProcessCommands(){
        while(true){
            WriteToSink(DOutSink, "> ");
            if(DCmdSrc->End()) break;

            std::string Line = ReadLine();
            auto Tokens = SplitTokens(Line);
            if(Tokens.empty()) continue;

            std::string Cmd = Tokens[0];

            if(Cmd == "exit"){
                break;
            }
            else if(Cmd == "help"){
                WriteToSink(DOutSink, "------------------------------------------------------------------------\n");
                WriteToSink(DOutSink, "help     Display this help menu\n");
                WriteToSink(DOutSink, "exit     Exit the program\n");
                WriteToSink(DOutSink, "count    Output the number of nodes in the map\n");
                WriteToSink(DOutSink, "node     Syntax \"node [0, count)\" \n");
                WriteToSink(DOutSink, "         Will output node ID and Lat/Lon for node\n");
                WriteToSink(DOutSink, "fastest  Syntax \"fastest start end\" \n");
                WriteToSink(DOutSink, "         Calculates the time for fastest path from start to end\n");
                WriteToSink(DOutSink, "shortest Syntax \"shortest start end\" \n");
                WriteToSink(DOutSink, "         Calculates the distance for the shortest path from start to end\n");
                WriteToSink(DOutSink, "save     Saves the last calculated path to file\n");
                WriteToSink(DOutSink, "print    Prints the steps for the last calculated path\n");
            }
            else if(Cmd == "count"){
                WriteToSink(DOutSink, std::to_string(DPlanner->NodeCount()) + " nodes\n");
            }
            else if(Cmd == "node"){
                if(Tokens.size() < 2){
                    WriteToSink(DErrSink, "Invalid node command, see help.\n");
                }
                else if(!IsUnsignedInt(Tokens[1])){
                    WriteToSink(DErrSink, "Invalid node parameter, see help.\n");
                }
                else{
                    std::size_t Index = std::stoul(Tokens[1]);
                    if(Index >= DPlanner->NodeCount()){
                        WriteToSink(DErrSink, "Invalid node parameter, see help.\n");
                    }
                    else{
                        auto Node = DPlanner->SortedNodeByIndex(Index);
                        auto DMS  = SGeographicUtils::ConvertLLToDMS(Node->Location());
                        WriteToSink(DOutSink, "Node " + std::to_string(Index) +
                                              ": id = " + std::to_string(Node->ID()) +
                                              " is at " + DMS + "\n");
                    }
                }
            }
            else if(Cmd == "shortest"){
                if(Tokens.size() < 3){
                    WriteToSink(DErrSink, "Invalid shortest command, see help.\n");
                }
                else if(!IsUnsignedInt(Tokens[1]) || !IsUnsignedInt(Tokens[2])){
                    WriteToSink(DErrSink, "Invalid shortest parameter, see help.\n");
                }
                else{
                    auto Src  = static_cast<CStreetMap::TNodeID>(std::stoul(Tokens[1]));
                    auto Dest = static_cast<CStreetMap::TNodeID>(std::stoul(Tokens[2]));
                    double Dist = DPlanner->FindShortestPath(Src, Dest, DLastShortestPath);

                    std::ostringstream OSS;
                    OSS << std::setprecision(10) << std::noshowpoint << Dist;
                    WriteToSink(DOutSink, "Shortest path is " + OSS.str() + " mi.\n");

                    DHasPath = true;
                    DLastWasFastest = false;
                    DLastDist = Dist;
                    DLastSrc  = Src;
                    DLastDest = Dest;
                }
            }
            else if(Cmd == "fastest"){
                if(Tokens.size() < 3){
                    WriteToSink(DErrSink, "Invalid fastest command, see help.\n");
                }
                else if(!IsUnsignedInt(Tokens[1]) || !IsUnsignedInt(Tokens[2])){
                    WriteToSink(DErrSink, "Invalid fastest parameter, see help.\n");
                }
                else{
                    auto Src  = static_cast<CStreetMap::TNodeID>(std::stoul(Tokens[1]));
                    auto Dest = static_cast<CStreetMap::TNodeID>(std::stoul(Tokens[2]));
                    double Time = DPlanner->FindFastestPath(Src, Dest, DLastFastestPath);

                    WriteToSink(DOutSink, "Fastest path takes " + FormatTime(Time) + ".\n");

                    DHasPath = true;
                    DLastWasFastest = true;
                    DLastTime = Time;
                    DLastSrc  = Src;
                    DLastDest = Dest;
                }
            }
            else if(Cmd == "save"){
                // save requires a fastest path to have been computed
                if(!DHasPath || !DLastWasFastest){
                    WriteToSink(DErrSink, "No valid path to save, see help.\n");
                }
                else{
                    // Build filename: "<src>_<dest>_<time>hr.csv"
                    std::ostringstream OSS;
                    OSS << DLastSrc << "_" << DLastDest << "_"
                        << std::fixed << std::setprecision(6) << DLastTime << "hr.csv";
                    std::string Filename = OSS.str();

                    auto Sink   = DResults->CreateSink(Filename);
                    auto Writer = std::make_shared<CDSVWriter>(Sink, ',', true);

                    Writer->WriteRow({"mode", "node_id"});
                    WriteToSink(Sink, "\n");

                    bool First = true;
                    for(auto &Step : DLastFastestPath){
                        if(!First) WriteToSink(Sink, "\n");
                        First = false;

                        std::string Mode;
                        if(Step.first == CTransportationPlanner::ETransportationMode::Walk)
                            Mode = "Walk";
                        else if(Step.first == CTransportationPlanner::ETransportationMode::Bike)
                            Mode = "Bike";
                        else
                            Mode = "Bus";

                        Writer->WriteRow({Mode, std::to_string(Step.second)});
                    }
                    WriteToSink(DOutSink, "Path saved to <results>/" + Filename + "\n");
                }
            }
            else if(Cmd == "print"){
                // print requires a fastest path to have been computed
                if(!DHasPath || !DLastWasFastest){
                    WriteToSink(DErrSink, "No valid path to print, see help.\n");
                }
                else{
                    std::vector<std::string> Desc;
                    DPlanner->GetPathDescription(DLastFastestPath, Desc);
                    for(auto &Line : Desc){
                        WriteToSink(DOutSink, Line + "\n");
                    }
                }
            }
            else{
                WriteToSink(DErrSink, "Unknown command \"" + Cmd + "\" type help for help.\n");
            }
        }
        return true;
    }
};

CTransportationPlannerCommandLine::CTransportationPlannerCommandLine(
    std::shared_ptr<CDataSource> cmdsrc,
    std::shared_ptr<CDataSink> outsink,
    std::shared_ptr<CDataSink> errsink,
    std::shared_ptr<CDataFactory> results,
    std::shared_ptr<CTransportationPlanner> planner)
{
    DImplementation = std::make_unique<SImplementation>(cmdsrc, outsink, errsink, results, planner);
}

CTransportationPlannerCommandLine::~CTransportationPlannerCommandLine(){
}

bool CTransportationPlannerCommandLine::ProcessCommands(){
    return DImplementation->ProcessCommands();
}
