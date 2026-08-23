# CStreetMap

## Overview

`CStreetMap` is the abstract base class for a street map. It defines the interface for querying nodes (points) and ways (sequences of nodes). Concrete implementations such as `COpenStreetMap` inherit from this class.

Defined in `include/StreetMap.h`.

---

## Types

| Type | Underlying type | Description |
|------|----------------|-------------|
| `TNodeID` | `uint64_t` | Unique identifier for a map node |
| `TWayID` | `uint64_t` | Unique identifier for a map way |
| `TLocation` | `std::pair<double, double>` | A geographic point as (latitude, longitude) |

## Constants

| Constant | Value | Description |
|----------|-------|-------------|
| `InvalidNodeID` | `UINT64_MAX` | Sentinel returned when a node is not found |
| `InvalidWayID` | `UINT64_MAX` | Sentinel returned when a way is not found |

---

## Inner Structs

### `SNode`

Represents a single point on the map (an OSM node). Each node has an ID, a geographic location, and zero or more key-value tags.

| Method | Returns | Description |
|--------|---------|-------------|
| `ID()` | `TNodeID` | The node's unique ID |
| `Location()` | `TLocation` | Geographic position as `(latitude, longitude)` |
| `AttributeCount()` | `std::size_t` | Number of tags on this node |
| `GetAttributeKey(index)` | `std::string` | Key of the tag at the given index; `""` if out of range |
| `HasAttribute(key)` | `bool` | `true` if a tag with the given key exists |
| `GetAttribute(key)` | `std::string` | Value of the tag; `""` if the key does not exist |

**Example — reading a node:**
```cpp
auto node = map->NodeByID(42);
if (node) {
    auto [lat, lon] = node->Location();
    std::cout << "Node " << node->ID()
              << " at (" << lat << ", " << lon << ")\n";

    for (std::size_t i = 0; i < node->AttributeCount(); i++) {
        std::string key = node->GetAttributeKey(i);
        std::cout << key << " = " << node->GetAttribute(key) << "\n";
    }
}
```

---

### `SWay`

Represents an ordered sequence of node references (an OSM way). Each way has an ID, a list of node IDs, and zero or more key-value tags.

| Method | Returns | Description |
|--------|---------|-------------|
| `ID()` | `TWayID` | The way's unique ID |
| `NodeCount()` | `std::size_t` | Number of node references in this way |
| `GetNodeID(index)` | `TNodeID` | Node ID at the given position; `InvalidNodeID` if out of range |
| `AttributeCount()` | `std::size_t` | Number of tags on this way |
| `GetAttributeKey(index)` | `std::string` | Key of the tag at the given index; `""` if out of range |
| `HasAttribute(key)` | `bool` | `true` if a tag with the given key exists |
| `GetAttribute(key)` | `std::string` | Value of the tag; `""` if the key does not exist |

**Example — reading a way:**
```cpp
auto way = map->WayByIndex(0);
if (way) {
    std::cout << "Way " << way->ID() << " has "
              << way->NodeCount() << " nodes\n";

    for (std::size_t i = 0; i < way->NodeCount(); i++) {
        std::cout << "  node ref: " << way->GetNodeID(i) << "\n";
    }

    if (way->HasAttribute("highway")) {
        std::cout << "Type: " << way->GetAttribute("highway") << "\n";
    }
}
```

---

## Virtual Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `NodeCount()` | `std::size_t` | Total number of nodes loaded |
| `WayCount()` | `std::size_t` | Total number of ways loaded |
| `NodeByIndex(index)` | `shared_ptr<SNode>` | Node at insertion-order position; `nullptr` if out of range |
| `NodeByID(id)` | `shared_ptr<SNode>` | Node with matching ID; `nullptr` if not found |
| `WayByIndex(index)` | `shared_ptr<SWay>` | Way at insertion-order position; `nullptr` if out of range |
| `WayByID(id)` | `shared_ptr<SWay>` | Way with matching ID; `nullptr` if not found |

**Example — iterating all nodes:**
```cpp
for (std::size_t i = 0; i < map->NodeCount(); i++) {
    auto node = map->NodeByIndex(i);
    auto [lat, lon] = node->Location();
    std::cout << node->ID() << ": " << lat << ", " << lon << "\n";
}
```

**Example — looking up a node by ID:**
```cpp
auto node = map->NodeByID(123456);
if (node == nullptr) {
    std::cout << "Node not found\n";
}
```
