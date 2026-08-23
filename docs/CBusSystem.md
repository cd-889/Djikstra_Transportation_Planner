# CBusSystem

## Overview

`CBusSystem` is the abstract base class for a bus system. It defines the interface for querying stops and routes. Concrete implementations (such as `CCSVBusSystem`) inherit from this class.

Defined in `include/BusSystem.h`.

---

## Types

| Type | Underlying type | Description |
|------|----------------|-------------|
| `TStopID` | `uint64_t` | Unique identifier for a bus stop |

## Constants

| Constant | Value | Description |
|----------|-------|-------------|
| `InvalidStopID` | `UINT64_MAX` | Sentinel returned when a stop is not found |

---

## Inner Structs

### `SStop`

Represents a single bus stop.

| Method | Returns | Description |
|--------|---------|-------------|
| `ID()` | `TStopID` | The stop's unique ID |
| `NodeID()` | `CStreetMap::TNodeID` | The street-map node this stop sits on |

### `SRoute`

Represents a named bus route as an ordered sequence of stop IDs.

| Method | Returns | Description |
|--------|---------|-------------|
| `Name()` | `std::string` | Route name (e.g. `"A"`, `"Route 1"`) |
| `StopCount()` | `std::size_t` | Number of stops in the route (duplicates counted) |
| `GetStopID(index)` | `TStopID` | Stop ID at the given position; returns `InvalidStopID` if out of range |

---

## Virtual Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `StopCount()` | `std::size_t` | Total number of stops loaded |
| `RouteCount()` | `std::size_t` | Total number of routes loaded |
| `StopByIndex(index)` | `shared_ptr<SStop>` | Stop at insertion-order position; `nullptr` if out of range |
| `StopByID(id)` | `shared_ptr<SStop>` | Stop with matching ID; `nullptr` if not found |
| `RouteByIndex(index)` | `shared_ptr<SRoute>` | Route at insertion-order position; `nullptr` if out of range |
| `RouteByName(name)` | `shared_ptr<SRoute>` | Route with matching name; `nullptr` if not found |
