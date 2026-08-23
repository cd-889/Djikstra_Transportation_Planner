# CTransportationPlanner

## Overview

`CTransportationPlanner` is an abstract base class that defines the interface for a multi-modal transportation route planner. It operates on a street map and bus system to find shortest (by distance) and fastest (by time) paths between nodes, supporting walking, biking, and bus travel modes.

Defined in: `include/TransportationPlanner.h`

---

## Types

### `TNodeID`
```cpp
using TNodeID = CStreetMap::TNodeID;
```
A `uint64_t` identifier for a node in the street map.

### `ETransportationMode`
```cpp
enum class ETransportationMode { Walk, Bike, Bus };
```
Enumerates the available modes of transportation:
- `Walk` — Traveling on foot.
- `Bike` — Traveling by bicycle.
- `Bus` — Traveling by bus.

### `TTripStep`
```cpp
using TTripStep = std::pair<ETransportationMode, TNodeID>;
```
Represents one step in a trip: the mode of transportation used to arrive at (or depart from) the node, and the node ID.

---

## Nested Struct: `SConfiguration`

The `SConfiguration` struct is an abstract interface that provides the planner with all configuration parameters. Concrete implementations (such as `STransportationPlannerConfig`) inherit from it.

```cpp
struct SConfiguration {
    virtual std::shared_ptr<CStreetMap> StreetMap() const noexcept = 0;
    virtual std::shared_ptr<CBusSystem> BusSystem() const noexcept = 0;
    virtual double WalkSpeed() const noexcept = 0;
    virtual double BikeSpeed() const noexcept = 0;
    virtual double DefaultSpeedLimit() const noexcept = 0;
    virtual double BusStopTime() const noexcept = 0;
    virtual int PrecomputeTime() const noexcept = 0;
};
```

| Method | Default | Description |
|---|---|---|
| `StreetMap()` | — | The OSM street map to plan routes on |
| `BusSystem()` | — | The bus system with stops and routes |
| `WalkSpeed()` | 3.0 mph | Walking speed |
| `BikeSpeed()` | 8.0 mph | Biking speed |
| `DefaultSpeedLimit()` | 25.0 mph | Speed limit for roads without a `maxspeed` tag |
| `BusStopTime()` | 30.0 sec | Time penalty added per bus stop |
| `PrecomputeTime()` | 30 sec | Maximum seconds allowed for precomputation |

---

## Member Functions

### `NodeCount`
```cpp
virtual std::size_t NodeCount() const noexcept = 0;
```
Returns the total number of nodes in the street map.

**Returns:** Number of nodes.

---

### `SortedNodeByIndex`
```cpp
virtual std::shared_ptr<CStreetMap::SNode> SortedNodeByIndex(std::size_t index) const noexcept = 0;
```
Returns the street map node at the given index, where nodes are sorted in ascending order by node ID.

**Parameters:**
- `index` — Zero-based index into the sorted node list.

**Returns:** Shared pointer to the node, or `nullptr` if `index >= NodeCount()`.

---

### `FindShortestPath`
```cpp
virtual double FindShortestPath(TNodeID src, TNodeID dest, std::vector<TNodeID> &path) = 0;
```
Finds the shortest path by distance (in miles) from `src` to `dest`. Respects one-way street constraints.

**Parameters:**
- `src` — Source node ID.
- `dest` — Destination node ID.
- `path` — Output vector filled with node IDs along the shortest path (inclusive of src and dest).

**Returns:** Total distance in miles, or `CPathRouter::NoPathExists` if no path exists.

---

### `FindFastestPath`
```cpp
virtual double FindFastestPath(TNodeID src, TNodeID dest, std::vector<TTripStep> &path) = 0;
```
Finds the fastest path by time (in hours) from `src` to `dest`, considering walking, biking, and bus travel. One-way constraints apply to bikes and buses; walking is always bidirectional.

**Parameters:**
- `src` — Source node ID.
- `dest` — Destination node ID.
- `path` — Output vector of `TTripStep` pairs (mode, node ID) describing the fastest path.

**Returns:** Total time in hours, or `CPathRouter::NoPathExists` if no path exists.

---

### `GetPathDescription`
```cpp
virtual bool GetPathDescription(const std::vector<TTripStep> &path, std::vector<std::string> &desc) const = 0;
```
Converts a trip step path into a human-readable list of directions.

**Parameters:**
- `path` — A vector of `TTripStep` values as returned by `FindFastestPath`.
- `desc` — Output vector of description strings (e.g., `"Walk N along Main St. for 1.2 mi"`).

**Returns:** `true` if the description was successfully generated.

---

## Usage Example

```cpp
#include "TransportationPlannerConfig.h"
#include "DijkstraTransportationPlanner.h"
#include "OpenStreetMap.h"
#include "CSVBusSystem.h"

// Set up street map and bus system (see COpenStreetMap and CCSVBusSystem docs)
auto config = std::make_shared<STransportationPlannerConfig>(streetMap, busSystem);
CDijkstraTransportationPlanner planner(config);

std::cout << "Nodes: " << planner.NodeCount() << std::endl;

std::vector<CTransportationPlanner::TNodeID> shortestPath;
double dist = planner.FindShortestPath(srcID, destID, shortestPath);

std::vector<CTransportationPlanner::TTripStep> fastestPath;
double time = planner.FindFastestPath(srcID, destID, fastestPath);

std::vector<std::string> desc;
planner.GetPathDescription(fastestPath, desc);
for (auto &line : desc) {
    std::cout << line << std::endl;
}
```
