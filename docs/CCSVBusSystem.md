# CCSVBusSystem

## Overview

`CCSVBusSystem` is a concrete implementation of `CBusSystem` that loads stop and route data from two CSV files via `CDSVReader`.

Defined in `include/CSVBusSystem.h`, implemented in `src/CSVBusSystem.cpp`.

See [CBusSystem.md](CBusSystem.md) for the base interface.

---

## Constructor

```cpp
CCSVBusSystem(std::shared_ptr<CDSVReader> stopsrc,
              std::shared_ptr<CDSVReader> routesrc);
```

| Parameter | Description |
|-----------|-------------|
| `stopsrc` | DSV reader for the stops CSV (must have columns `stop_id`, `node_id`) |
| `routesrc` | DSV reader for the routes CSV (must have columns `route`, `stop_id`) |

Both readers are consumed once during construction. The header row is located automatically regardless of column order.

---

## CSV Format

**Stops file**
```
stop_id,node_id
1,123
2,124
```

**Routes file**
```
route,stop_id
A,1
A,2
B,2
B,3
```

### Rules
- Rows with the same `route` value must be **consecutive** and are grouped into a single route in the order they appear.
- A stop ID may appear more than once in a route (e.g. a loop).
- Malformed rows (non-numeric IDs, missing columns) are silently skipped.

---

## Internal Design

Uses the **pimpl idiom** (`SImplementation`) to hide data members from the header.

Each collection is stored in two parallel structures:

| Structure | Purpose |
|-----------|---------|
| `std::vector` | Preserves insertion order for `ByIndex` lookups — O(1) |
| `std::unordered_map` | Enables O(1) average `ByID` / `ByName` lookups |

---

## Tests

Tests live in `testsrc/CSVBusSystemTest.cpp`. Run them with:

```bash
make directories && make run_csvbstest
# or, after a full build:
./testbin/testcsvbs
```

| Test | What it covers |
|------|---------------|
| `SimpleFiles` | Stop count, `StopByIndex`, `StopByID`, `ID()`, `NodeID()` |
| `RouteCountAndRouteByName` | Route count, `RouteByName`, stop order within routes |
| `RouteByIndex` | Routes returned in CSV insertion order |
| `StopByIndexOutOfRange` | `StopByIndex` returns `nullptr` for index >= StopCount |
| `StopByIDNotFound` | `StopByID` returns `nullptr` for unknown ID |
| `RouteByIndexOutOfRange` | `RouteByIndex` returns `nullptr` for index >= RouteCount |
| `RouteByNameNotFound` | `RouteByName` returns `nullptr` for unknown or empty name |
| `RouteGetStopIDOutOfRange` | `GetStopID` returns `InvalidStopID` for out-of-range index |
| `RouteWithRepeatedStop` | Loop routes where a stop ID appears multiple times |
| `SingleStopSingleRoute` | Edge case with exactly one stop and one route |
