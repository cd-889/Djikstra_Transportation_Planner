# CPathRouter

## Overview

`CPathRouter` is an abstract base class that defines the interface for a generic weighted graph path router. Concrete implementations (such as `CDijkstraPathRouter`) inherit from this class and provide the actual shortest-path algorithm. The class supports tagged vertices and weighted directed (or bidirectional) edges.

Defined in: `include/PathRouter.h`

---

## Types

### `TVertexID`
```cpp
using TVertexID = std::size_t;
```
Represents a vertex identifier. Vertex IDs are assigned sequentially starting from `0` when vertices are added via `AddVertex`.

---

## Constants

### `InvalidVertexID`
```cpp
static constexpr TVertexID InvalidVertexID = std::numeric_limits<TVertexID>::max();
```
Sentinel value returned when a vertex ID is not valid.

### `NoPathExists`
```cpp
static constexpr double NoPathExists = std::numeric_limits<double>::max();
```
Sentinel value returned by `FindShortestPath` when no path exists between the source and destination vertices.

---

## Member Functions

### `VertexCount`
```cpp
virtual std::size_t VertexCount() const noexcept = 0;
```
Returns the total number of vertices currently in the router.

**Returns:** Number of vertices.

---

### `AddVertex`
```cpp
virtual TVertexID AddVertex(std::any tag) noexcept = 0;
```
Adds a new vertex to the router with an associated tag. The tag can hold any type via `std::any` (e.g., a node ID, a string label, etc.).

**Parameters:**
- `tag` — An arbitrary value associated with this vertex.

**Returns:** The `TVertexID` assigned to the new vertex.

---

### `GetVertexTag`
```cpp
virtual std::any GetVertexTag(TVertexID id) const noexcept = 0;
```
Retrieves the tag associated with the given vertex ID.

**Parameters:**
- `id` — The vertex ID to look up.

**Returns:** The `std::any` tag stored for that vertex, or an empty `std::any()` if `id` is invalid.

---

### `AddEdge`
```cpp
virtual bool AddEdge(TVertexID src, TVertexID dest, double weight, bool bidir = false) noexcept = 0;
```
Adds a directed edge from `src` to `dest` with the given weight. If `bidir` is `true`, a reverse edge from `dest` to `src` with the same weight is also added.

**Parameters:**
- `src` — Source vertex ID.
- `dest` — Destination vertex ID.
- `weight` — Non-negative edge weight.
- `bidir` — If `true`, adds edges in both directions (default: `false`).

**Returns:** `true` if the edge was added successfully; `false` if either vertex does not exist or the weight is negative.

---

### `Precompute`
```cpp
virtual bool Precompute(std::chrono::steady_clock::time_point deadline) noexcept = 0;
```
Allows the router to perform optional precomputation (e.g., all-pairs shortest paths) before queries are made. The implementation must stop precomputing by the given deadline.

**Parameters:**
- `deadline` — A time point by which precomputation must finish.

**Returns:** `true` if precomputation completed successfully.

---

### `FindShortestPath`
```cpp
virtual double FindShortestPath(TVertexID src, TVertexID dest, std::vector<TVertexID> &path) noexcept = 0;
```
Finds the shortest path from `src` to `dest` and fills `path` with the sequence of vertex IDs from source to destination (inclusive).

**Parameters:**
- `src` — Source vertex ID.
- `dest` — Destination vertex ID.
- `path` — Output vector filled with the vertex IDs along the shortest path.

**Returns:** Total path weight (distance), or `NoPathExists` if no path exists. `path` is cleared if no path exists.

---

## Usage Example

```cpp
#include "DijkstraPathRouter.h"
#include <iostream>

int main() {
    CDijkstraPathRouter router;

    auto A = router.AddVertex(std::string("A"));
    auto B = router.AddVertex(std::string("B"));
    auto C = router.AddVertex(std::string("C"));

    router.AddEdge(A, B, 1.0);
    router.AddEdge(B, C, 2.0);
    router.AddEdge(A, C, 5.0);

    std::vector<CPathRouter::TVertexID> path;
    double dist = router.FindShortestPath(A, C, path);
    // dist == 3.0, path == {A, B, C}

    for (auto id : path) {
        std::cout << std::any_cast<std::string>(router.GetVertexTag(id)) << " ";
    }
    // Output: A B C
}
```
