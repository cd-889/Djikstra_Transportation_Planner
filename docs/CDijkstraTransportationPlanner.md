# CDijkstraTransportationPlanner

## Overview

`CDijkstraTransportationPlanner` is a concrete implementation of the `CTransportationPlanner` abstract interface. It uses Dijkstra's Algorithm to compute both the shortest path (by distance) and the fastest path (by time) between nodes on a street map, incorporating walking, biking, and bus travel modes.

Defined in: `include/DijkstraTransportationPlanner.h`  
Implemented in: `src/DijkstraTransportationPlanner.cpp`

Inherits from: [`CTransportationPlanner`](CTransportationPlanner.md)

---

## Key Assumptions

- **Shortest path**: Follows one-way street constraints (`oneway=yes` tag). Distance is in miles using the Haversine formula.
- **Fastest path**: Walk edges are always bidirectional. Bike and bus edges respect `oneway`. Roads tagged `bicycle=no` cannot be biked.
- **Bus speed**: Uses the road's `maxspeed` tag, or `DefaultSpeedLimit()` from config if not specified (default 25 mph).
- **Bus stop time**: A time penalty of `BusStopTime() / 3600` hours is added per bus segment (default 30 seconds).
- **Walk speed**: From `WalkSpeed()` in config (default 3 mph).
- **Bike speed**: From `BikeSpeed()` in config (default 8 mph).
- **You cannot take a bike on the bus**: If the fastest path uses the bus, you must walk to the bus stop.
- Nodes are sorted by ascending node ID for `SortedNodeByIndex`.

---

## Constructor / Destructor

### `CDijkstraTransportationPlanner(std::shared_ptr<SConfiguration> config)`
```cpp
CDijkstraTransportationPlanner(std::shared_ptr<SConfiguration> config);
```
Constructs the planner using the provided configuration. During construction, the planner:
1. Sorts all street map nodes by ID.
2. Builds a distance-based graph for `FindShortestPath`.
3. Builds a time-based adjacency list with Walk, Bike, and Bus edges for `FindFastestPath`.
4. Calls `Precompute` on the shortest-path router within the configured time limit.

**Parameters:**
- `config` — A shared pointer to an `SConfiguration` implementation (e.g., `STransportationPlannerConfig`).

### `~CDijkstraTransportationPlanner()`
```cpp
~CDijkstraTransportationPlanner();
```
Destroys the planner and frees all internal resources.

---

## Member Functions

### `NodeCount`
```cpp
std::size_t NodeCount() const noexcept override;
```
Returns the number of nodes in the street map.

**Returns:** Total node count.

**Example:**
```cpp
CDijkstraTransportationPlanner planner(config);
std::cout << planner.NodeCount() << std::endl;  // e.g., 4
```

---

### `SortedNodeByIndex`
```cpp
std::shared_ptr<CStreetMap::SNode> SortedNodeByIndex(std::size_t index) const noexcept override;
```
Returns the node at the given index in the list of nodes sorted by ascending node ID.

**Parameters:**
- `index` — Zero-based index.

**Returns:** Shared pointer to the node, or `nullptr` if `index >= NodeCount()`.

**Example:**
```cpp
for (std::size_t i = 0; i < planner.NodeCount(); i++) {
    auto node = planner.SortedNodeByIndex(i);
    std::cout << "Node " << i << ": ID = " << node->ID() << std::endl;
}
```

---

### `FindShortestPath`
```cpp
double FindShortestPath(TNodeID src, TNodeID dest, std::vector<TNodeID> &path) override;
```
Finds the shortest path by distance (miles) from `src` to `dest` using Dijkstra's Algorithm on the street graph. One-way constraints are respected.

**Parameters:**
- `src` — Source node ID.
- `dest` — Destination node ID.
- `path` — Output vector of node IDs from source to destination.

**Returns:** Distance in miles, or `CPathRouter::NoPathExists` if unreachable.

**Example:**
```cpp
std::vector<CTransportationPlanner::TNodeID> path;
double dist = planner.FindShortestPath(1, 4, path);
// dist == total miles
// path == {1, 2, 3, 4}
```

---

### `FindFastestPath`
```cpp
double FindFastestPath(TNodeID src, TNodeID dest, std::vector<TTripStep> &path) override;
```
Finds the fastest path by time (hours) from `src` to `dest`, considering all transportation modes. The result path uses `TTripStep = {ETransportationMode, TNodeID}`.

The mode at each step indicates how you arrived at that node from the previous node. The first node's mode is `Walk` if the first edge is a bus edge (you walk to the bus stop), otherwise it matches the first edge's mode.

**Parameters:**
- `src` — Source node ID.
- `dest` — Destination node ID.
- `path` — Output vector of `TTripStep` pairs.

**Returns:** Time in hours, or `CPathRouter::NoPathExists` if unreachable.

**Example:**
```cpp
std::vector<CTransportationPlanner::TTripStep> path;
double time = planner.FindFastestPath(1, 3, path);
// e.g., path == {{Walk,1}, {Bus,2}, {Bus,3}}
// time == distance/speed + bus_stop_times
```

---

### `GetPathDescription`
```cpp
bool GetPathDescription(const std::vector<TTripStep> &path, std::vector<std::string> &desc) const override;
```
Converts a `TTripStep` path into a human-readable list of directions. Each entry in `desc` describes one segment of the trip.

**Description format:**
- First entry: `"Start at <DMS coordinates>"`
- Walk/Bike on named road: `"Walk/Bike <direction> along <road name> for <dist> mi"`
- Walk/Bike on unnamed road: `"Walk/Bike <direction> toward <next road name> for <dist> mi"` or `"... toward End"`
- Bus segment: `"Take Bus <route> from stop <id> to stop <id>"`
- Last entry: `"End at <DMS coordinates>"`

Consecutive steps on the same road and same mode are merged into a single description entry.

**Parameters:**
- `path` — A `TTripStep` vector as returned by `FindFastestPath`.
- `desc` — Output vector of human-readable direction strings.

**Returns:** `true` if the description was generated successfully.

**Example:**
```cpp
std::vector<CTransportationPlanner::TTripStep> path = {
    {CTransportationPlanner::ETransportationMode::Walk, 8},
    {CTransportationPlanner::ETransportationMode::Walk, 1},
    {CTransportationPlanner::ETransportationMode::Bus,  3},
    {CTransportationPlanner::ETransportationMode::Bus,  5}
};
std::vector<std::string> desc;
planner.GetPathDescription(path, desc);
// desc[0] == "Start at 38d 30' 0\" N, 121d 43' 12\" W"
// desc[1] == "Walk E along Main St. for 1.1 mi"
// desc[2] == "Take Bus A from stop 101 to stop 103"
// desc[3] == "End at 38d 36' 0\" N, 121d 47' 60\" W"
```

---

## Full Usage Example

```cpp
#include "TransportationPlannerConfig.h"
#include "DijkstraTransportationPlanner.h"
#include "OpenStreetMap.h"
#include "CSVBusSystem.h"
#include "DSVReader.h"
#include "XMLReader.h"
#include "FileDataFactory.h"
#include <iostream>

int main() {
    auto factory = std::make_shared<CFileDataFactory>("./data");

    auto xmlReader  = std::make_shared<CXMLReader>(factory->CreateSource("city.osm"));
    auto stopReader = std::make_shared<CDSVReader>(factory->CreateSource("stops.csv"), ',');
    auto routeReader= std::make_shared<CDSVReader>(factory->CreateSource("routes.csv"), ',');

    auto streetMap = std::make_shared<COpenStreetMap>(xmlReader);
    auto busSystem = std::make_shared<CCSVBusSystem>(stopReader, routeReader);
    auto config    = std::make_shared<STransportationPlannerConfig>(streetMap, busSystem);

    CDijkstraTransportationPlanner planner(config);

    std::cout << "Total nodes: " << planner.NodeCount() << std::endl;

    // Use the first and last sorted nodes as source and destination
    auto srcNode  = planner.SortedNodeByIndex(0);
    auto destNode = planner.SortedNodeByIndex(planner.NodeCount() - 1);
    CTransportationPlanner::TNodeID srcID  = srcNode->ID();
    CTransportationPlanner::TNodeID destID = destNode->ID();

    // Shortest path
    std::vector<CTransportationPlanner::TNodeID> shortPath;
    double dist = planner.FindShortestPath(srcID, destID, shortPath);
    std::cout << "Shortest: " << dist << " mi" << std::endl;

    // Fastest path
    std::vector<CTransportationPlanner::TTripStep> fastPath;
    double time = planner.FindFastestPath(srcID, destID, fastPath);
    std::cout << "Fastest: " << time * 60.0 << " min" << std::endl;

    // Human-readable directions
    std::vector<std::string> desc;
    planner.GetPathDescription(fastPath, desc);
    for (auto &line : desc) {
        std::cout << line << std::endl;
    }
    return 0;
}
```
