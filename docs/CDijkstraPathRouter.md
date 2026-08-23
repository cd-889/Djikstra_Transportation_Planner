# CDijkstraPathRouter

## Overview

`CDijkstraPathRouter` is a concrete implementation of the `CPathRouter` abstract interface. It finds the shortest path between two vertices using **Dijkstra's Algorithm** with a min-heap priority queue. Vertices hold arbitrary tags via `std::any`, and edges are weighted directed connections between vertices.

Defined in: `include/DijkstraPathRouter.h`  
Implemented in: `src/DijkstraPathRouter.cpp`

Inherits from: [`CPathRouter`](CPathRouter.md)

---

## Constructor / Destructor

### `CDijkstraPathRouter()`
```cpp
CDijkstraPathRouter();
```
Constructs an empty path router with no vertices or edges.

### `~CDijkstraPathRouter()`
```cpp
~CDijkstraPathRouter();
```
Destroys the path router and frees all internal resources.

---

## Member Functions

### `VertexCount`
```cpp
std::size_t VertexCount() const noexcept;
```
Returns the number of vertices currently in the router.

**Returns:** Number of vertices.

**Example:**
```cpp
CDijkstraPathRouter router;
// router.VertexCount() == 0
router.AddVertex(std::string("A"));
// router.VertexCount() == 1
```

---

### `AddVertex`
```cpp
TVertexID AddVertex(std::any tag) noexcept;
```
Adds a new vertex with an associated tag. Vertex IDs are assigned sequentially starting from `0`.

**Parameters:**
- `tag` — Any value to associate with the vertex (e.g., a node ID, label string, etc.).

**Returns:** The `TVertexID` of the newly created vertex.

**Example:**
```cpp
CDijkstraPathRouter router;
auto idA = router.AddVertex(std::string("A"));  // idA == 0
auto idB = router.AddVertex(std::string("B"));  // idB == 1
auto idC = router.AddVertex(uint64_t(42));       // idC == 2
```

---

### `GetVertexTag`
```cpp
std::any GetVertexTag(TVertexID id) const noexcept;
```
Retrieves the tag stored for the given vertex.

**Parameters:**
- `id` — The vertex ID to query.

**Returns:** The `std::any` tag for the vertex, or an empty `std::any()` if `id` is out of range.

**Example:**
```cpp
auto id = router.AddVertex(std::string("hello"));
auto tag = std::any_cast<std::string>(router.GetVertexTag(id));
// tag == "hello"
```

---

### `AddEdge`
```cpp
bool AddEdge(TVertexID src, TVertexID dest, double weight, bool bidir = false) noexcept;
```
Adds a directed edge from `src` to `dest`. If `bidir` is `true`, a reverse edge is also added.

**Parameters:**
- `src` — Source vertex ID.
- `dest` — Destination vertex ID.
- `weight` — Edge weight (must be non-negative).
- `bidir` — If `true`, adds edges in both directions (default: `false`).

**Returns:** `true` on success; `false` if either vertex ID is invalid.

**Example:**
```cpp
auto A = router.AddVertex(std::string("A"));
auto B = router.AddVertex(std::string("B"));
auto C = router.AddVertex(std::string("C"));
router.AddEdge(A, B, 4.0);        // directed A -> B
router.AddEdge(B, C, 2.0, true);  // bidirectional B <-> C
router.AddEdge(A, C, 7.0);        // directed A -> C
```

---

### `Precompute`
```cpp
bool Precompute(std::chrono::steady_clock::time_point deadline) noexcept;
```
Performs optional precomputation before path queries. The current implementation returns immediately (no precomputation is done), but the deadline is respected.

**Parameters:**
- `deadline` — Time point by which any precomputation must finish.

**Returns:** Always `true`.

---

### `FindShortestPath`
```cpp
double FindShortestPath(TVertexID src, TVertexID dest, std::vector<TVertexID> &path) noexcept;
```
Finds the shortest path from `src` to `dest` using Dijkstra's Algorithm. The `path` vector is filled with vertex IDs from source to destination (inclusive).

**Parameters:**
- `src` — Source vertex ID.
- `dest` — Destination vertex ID.
- `path` — Output vector of vertex IDs along the shortest path.

**Returns:** Total path weight, or `CPathRouter::NoPathExists` if no path exists. `path` is cleared if no path is found.

**Example:**
```cpp
CDijkstraPathRouter router;
/*
       5
  A ---> B ---> C
    4   2|    ^
         V   / 1
          D
*/
auto A = router.AddVertex(std::string("A"));
auto B = router.AddVertex(std::string("B"));
auto C = router.AddVertex(std::string("C"));
auto D = router.AddVertex(std::string("D"));

router.AddEdge(A, B, 4.0);
router.AddEdge(B, C, 5.0);
router.AddEdge(B, D, 2.0);
router.AddEdge(D, C, 1.0);

std::vector<CPathRouter::TVertexID> path;
double dist = router.FindShortestPath(A, C, path);
// dist == 7.0
// path == {A, B, D, C}
```

---

## Full Usage Example

```cpp
#include "DijkstraPathRouter.h"
#include <iostream>
#include <any>

int main() {
    CDijkstraPathRouter router;

    auto v0 = router.AddVertex(std::string("Start"));
    auto v1 = router.AddVertex(std::string("Mid"));
    auto v2 = router.AddVertex(std::string("End"));

    router.AddEdge(v0, v1, 3.0);
    router.AddEdge(v1, v2, 2.0);
    router.AddEdge(v0, v2, 10.0);

    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    router.Precompute(deadline);

    std::vector<CPathRouter::TVertexID> path;
    double dist = router.FindShortestPath(v0, v2, path);

    std::cout << "Distance: " << dist << std::endl;  // 5.0
    for (auto id : path) {
        std::cout << std::any_cast<std::string>(router.GetVertexTag(id)) << " ";
    }
    // Output: Start Mid End
}
```
