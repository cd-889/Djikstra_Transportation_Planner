# CTransportationPlannerCommandLine

## Overview

`CTransportationPlannerCommandLine` is a concrete implementation of a command line interface, made for interacting with the transportation planner. It is meant to read commands from a data source and process them using a `CTransportationPlanner` and writes the results to output and error sinks. 

Defined in: `include/TransportationPlannerCommandLine.h`  
Implemented in: `src/TransportationPlannerCommandLine.cpp`

---

## Constructor / Destructor

### `CTransportationPlannerCommandLine()`
```cpp
CTransportationPlannerCommandLine(
    std::shared_ptr<CDataSource> cmdsrc,
    std::shared_ptr<CDataSink> outsink,
    std::shared_ptr<CDataSink> errsink,
    std::shared_ptr<CDataFactory> results,
    std::shared_ptr<CTransportationPlanner> planner
);
```
Constructs a command-line interface in order to bring together the input/output processes and transportation planning components. 

**Parameters:**
- `cmdsrc` — This is the data source to read commands from. 
- `outsink`- This is the data sink for normal output. This is where the outputs will go. 
- `errsink` - This is the data sink for all the error messages. This is where errors will go. 
- `results`- This is the data factory used to create output files. 
- `planner` - This is the transportational planner used to calculate the paths. 

All of these parameters rely on the DataSink, DataSource, OSM, CSVBusSystem and File Systems for processing through `transplanning.cpp`

### `~CTransportationPlannerCommandLine()`
```cpp
~CTransportationPlannerCommandLine();
```
This destroys the command-line-interface and frees all the internal processes to process commands through the command line. 


## Member Functions

### `ProcessCommands`
```cpp
bool ProcessCommands(); 
```
This is the function that will loop through the commands and reads and processes commands from the data source. This continues until `exit` is reached or the source is exhausted. It will also print a ` ">" ` prompt before the start of each command to simulate the command line experience.  
**Returns:** true when the command loop completes fully. 

## Supported Commands

### help

This prints a help menu to show all the available commands and their syntax to the output sink. 


```
help     Display this help menu
exit     Exit the program
count    Output the number of nodes in the map
node     Syntax "node [0, count)" 
          Will output node ID and Lat/Lon for node
fastest  Syntax "fastest start end" 
         Calculates the time for fastest path from start to end
shortest Syntax "shortest start end" 
         Calculates the distance for the shortest path from start to end
save     Saves the last calculated path to file
print    Prints the steps for the last calculated path
```

### exit
This will exit the command loop immediately. 

### count
This will output the total number of nodes in the street map. 

#### Output format - `<N> nodes`

### node <index>
This outputs the ID and geographic location of the node at the given index. 

#### Parameters

- `index` - this is an integer from `[0,count]`

#### Output Format - `Node <index>: id = <id> is at <DMS location>`

#### Errors

- Missing parameter - Will output `"Invalid node command, see help.\n"` to the error sink.
- Non-integer or out of range - Will output `"Invalid node command, see help.\n"` to the error sink.

### shortest <src> <dest>
It will find and output the shortest path distance between two node IDs. 

#### Parameters

- src - This is the source node ID. 
- destination - This is the destination node ID. 

#### Output Format - `Shortest path is <distance> mi`

#### Errors

- Missing parameter - Will output `"Invalid shortest command, see help.\n"` to the error sink.
- Non-integer parameter - Will output `"Invalid shortest parameter, see help.\n"` to the error sink.

### fastest <src> <dest>
It will find and output the fastest path distance between two node IDs. 

#### Parameters

- src - This is the source node ID. 
- destination - This is the destination node ID. 

#### Output Format - `Fastest path takes <time>`

`<time>` is formatted in the following manner: `"3 hr 30 min 15 sec"` or `"40 min"`

#### Errors

- Missing parameter - Will output `"Invalid fastest command, see help.\n"` to the error sink.
- Non-integer parameter - Will output `"Invalid fastest parameter, see help.\n"` to the error sink.

### save 
It will save the last calculated fastest path to a CSV file in the results directory. 

It requires that a fastest path be computed with `fastest` first. 

#### Filename format: `<src><dest>_<time>hr.csv`

#### CSV format 

```
mode, node_id
Walk, 5
Bike, 4
Bus, 2
```

#### Output Format: `Path saved to <results>/<filename>`

#### Errors

- No valid fastest path found -> `"No valid path to save, see help.\n"` to the error sink. 


### print
It will print the step-by-step description of the last calculated fastest path. 

It requires a fastest path to be computed with `fastest` first. 

#### Output format: 

```
Start at 24d 13' 70" S, 145d 24' 17" E
Walk towards Central Avenue. for 5.4 mi
....
End at 24d 45' 70" S, 145d 45' 50" W
```


#### Errors
- No valid fastest path found -> `"No valid path to save, see help.\n"` to the error sink. 

#### Unknown Command
This includes any unrecognized command and will cause an error. 

Example: 
```
Unknown command "<cmd>" type help for help. 
```

#### Helper Functions
There are four helpers that help support the above commands. 

`WriteToSink` 
``` cpp
void WriteToSink(std::shared_ptr<CDataSink> sink, const std::string &str);
``` 

This will write a string to a data sink character by character using `sink->Put()`

`ReadLine`

``` cpp
std::string ReadLine(); 
```

This will read characters from `DCmdSrc` one at a time until it encounters a new line character or the source is exhausted. It returns the accumulated line as a string.

`SplitTokens` 

```cpp
std::vector<std::string> SplitTokens(const std::string &line); 
```

This will split a string into whitespace-delimited tokens using `std::istringstream`. It will return a vector of token strings. 

`FormatTime` 

```cpp
std::string FormatTime(double hours);
```

This will convert a time in hours represented as a double into a human-friendly representation of time as a string. 

##### Examples
- `0.0` -> `"0 min"`
- `1.25` -> `"1 hr 15 min"`

The non-zero time components are included. 

---
## Example Usage
```cpp
#include "TransportationPlannerCommandLine.h"
#include "StandardDataSource.h"
#include "StandardDataSink.h"
#include "StandardErrorDataSink.h"
#include "FileDataFactory.h"
// Planner is already constructed according to the appropriate process
auto input = std::make_shared<CStandardDataSource>(); 
auto output = std::make_shared<CStandardDataSink>();
auto error = std::make_shared<CStandardErrorDataSink>(); 
auto results = std::make_shared<CFileDataFactory>("./results");

auto cmdline = std::make_shared<CTransportationPlannerCommandLine>(input,output, error, results, planner);
cmdline->ProcessCommands();
```
---

### Example session

```
> count
3 nodes
> shortest 421 893
Shortest path is 5.6 mi.
> fastest 421 893
Fastest path takes 30 min.
> save
Path saved to </results>/421_893_0.50000hr.csv
>exit
```



