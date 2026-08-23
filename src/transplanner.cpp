#include "TransportationPlannerConfig.h"
#include "DijkstraTransportationPlanner.h"
#include "TransportationPlannerCommandLine.h"
#include "OpenStreetMap.h"
#include "CSVBusSystem.h"
#include "FileDataFactory.h"
#include "StandardDataSource.h"
#include "StandardDataSink.h"
#include "StandardErrorDataSink.h"
#include "DSVReader.h"
#include "XMLReader.h"
#include <iostream>
#include <string>
#include <vector>

void PrintSyntax(){
    // TODO: print usage/syntax error message to stderr
    std::cerr << "Usage: ./transport [--data=<path>] [--results=<path>]" << std::endl; 
    std::cerr << "--data=<path>" << "will set the data directory" << std::endl; 
    std::cerr << "--results=<path>" << "will set the results directory" << std::endl; 
    
}

int main(int argc, char *argv[]){
    std::string DataDirectory = "./data";
    std::string ResultsDirectory = "./results";
    bool ArgumentsValid = true;

    // TODO: parse command-line arguments
    // Supported flags:
    //   --data=<path>     sets DataDirectory
    //   --results=<path>  sets ResultsDirectory
    // Set ArgumentsValid = false for unrecognized flags

    for (auto i = 1; i < argc; i++)
    {
        std::string argument = argv[i]; 

        if (argument.find("--data=") == 0)
        {
            DataDirectory = argument.substr(7); 
        }
        else if (argument.find("--results=") == 0)
        {
            ResultsDirectory = argument.substr(10); 
        }
        else 
        {
            ArgumentsValid = false; 
        }
    }

    if(!ArgumentsValid){
        PrintSyntax();
        return EXIT_FAILURE;
    }

    const std::string OSMFilename   = "city.osm";
    const std::string StopFilename  = "stops.csv";
    const std::string RouteFilename = "routes.csv";

    // TODO: create CFileDataFactory for DataDirectory and ResultsDirectory
    auto DataFactory = std::make_shared<CFileDataFactory>(DataDirectory); 
    auto ResultsFactory = std::make_shared<CFileDataFactory>(ResultsDirectory);
    // TODO: open data sources for OSM, stops, and routes CSV files
    auto OSM_source = DataFactory -> CreateSource(OSMFilename); 
    if (!OSM_source)
    {
        std::cerr << "Failure opening " << OSMFilename << std::endl;
        return EXIT_FAILURE;  
    }
    auto stop_source = DataFactory -> CreateSource(StopFilename); 
    if (!stop_source)
    {
        std::cerr << "Failure opening " << StopFilename << std::endl;
        return EXIT_FAILURE;  
    }
    auto route_source = DataFactory -> CreateSource(RouteFilename); 
     if (!route_source)
    {
        std::cerr << "Failure opening " << RouteFilename << std::endl;
        return EXIT_FAILURE;  
    }
    // Return EXIT_FAILURE with an error message if any source fails to open
    
    // TODO: create CXMLReader, CDSVReader (stops), CDSVReader (routes)
    auto OSM_reader = std::make_shared<CXMLReader> (OSM_source);
    auto stop_reader = std::make_shared<CDSVReader> (stop_source, ',');
    auto route_reader = std::make_shared<CDSVReader> (route_source, ',');
    // TODO: create COpenStreetMap, CCSVBusSystem, STransportationPlannerConfig,
    //       and CDijkstraTransportationPlanner

    auto OSM = std::make_shared<COpenStreetMap> (OSM_reader); 
    auto stops = std::make_shared<CCSVBusSystem> (stop_reader, route_reader); 
    auto transport = std::make_shared <STransportationPlannerConfig> (OSM, stops); 
    auto planner = std::make_shared <CDijkstraTransportationPlanner> (transport); 


    // TODO: create CStandardDataSource, CStandardDataSink, CStandardErrorDataSink
    auto in = std::make_shared<CStandardDataSource>();
    auto out = std::make_shared<CStandardDataSink>();
    auto error = std::make_shared<CStandardErrorDataSink>();

    // TODO: instantiate CTransportationPlannerCommandLine and call ProcessCommands()
    auto cmd = std::make_shared<CTransportationPlannerCommandLine>(in,out,error,ResultsFactory, planner);
    cmd->ProcessCommands(); 


    return EXIT_SUCCESS;
}
