# COpenStreetMap


## Overview

`COpenStreetMap` is a concrete implementation of `CStreetMap` that parses an OpenStreetMap XML file via `CXMLReader` and exposes its nodes and ways for lookup.

Defined in `include/OpenStreetMap.h`, implemented in `src/OpenStreetMap.cpp`.

See [CStreetMap.md](CStreetMap.md) for the base interface.

---

## Constructor

```cpp
COpenStreetMap(std::shared_ptr<CXMLReader> src);
```

| Parameter | Description |
|-----------|-------------|
| `src` | XML reader wrapping an OSM-formatted data source |

The entire XML is parsed once during construction. After the constructor returns, all nodes and ways are available via the lookup methods.

**Example:**
```cpp
auto DataSource = std::make_shared<CStringDataSource>(osmXmlString);
auto XMLReader  = std::make_shared<CXMLReader>(DataSource);
COpenStreetMap map(XMLReader);

std::cout << map.NodeCount() << " nodes\n";
std::cout << map.WayCount()  << " ways\n";
```

---

## OSM XML Format

```xml
<?xml version='1.0' encoding='UTF-8'?>
<osm version="0.6">
  <node id="1" lat="38.5" lon="-121.7"/>
  <node id="2" lat="38.5" lon="-121.8"/>
  <way id="100">
    <nd ref="1"/>
    <nd ref="2"/>
  </way>
</osm>
```

### Rules
- `<node>` elements are self-closing — the XML reader emits them as `CompleteElement`.
- `<way>` elements use `StartElement` / `EndElement` pairs and contain `<nd ref="..."/>` children (also self-closing).
- All nodes appear before all ways in the file.

---

## Lookup Methods

All methods are inherited from `CStreetMap`. See [CStreetMap.md](CStreetMap.md) for the full method table and examples.

**Example — find a node and check its location:**
```cpp
auto node = map.NodeByIndex(0);
if (node) {
    auto [lat, lon] = node->Location();
    std::cout << "ID: " << node->ID()
              << " lat: " << lat
              << " lon: " << lon << "\n";
}
```

**Example — look up the same node by ID:**
```cpp
auto node = map.NodeByID(1);
if (node) {
    std::cout << "Found node " << node->ID() << "\n";
}
```

**Example — iterate all ways:**
```cpp
for (std::size_t i = 0; i < map.WayCount(); i++) {
    auto way = map.WayByIndex(i);
    std::cout << "Way " << way->ID()
              << " has " << way->NodeCount() << " node refs\n";
}
```

---

## Internal Design (SImplementation)

Uses the **pimpl idiom** — all data and parsing logic live in `SImplementation` inside the `.cpp` file.

### Data structures

| Member | Type | Purpose |
|--------|------|---------|
| `DNodesByIndex` | `vector<shared_ptr<SNode>>` | Nodes in insertion order for `NodeByIndex` |
| `DNodesByID` | `unordered_map<TNodeID, shared_ptr<SNode>>` | O(1) lookup by node ID |
| `DWaysByIndex` | `vector<shared_ptr<SWay>>` | Ways in insertion order for `WayByIndex` |
| `DWaysByID` | `unordered_map<TWayID, shared_ptr<SWay>>` | O(1) lookup by way ID |

### Parsing flow

```
ParseOpenStreetMap
  └─ reads <osm> start element
      ├─ ParseNodes — fills DNodesByIndex, DNodesByID
      │    └─ on first non-node entity, passes it back to:
      └─ ParseWays  — fills DWaysByIndex, DWaysByID
```

---



## Tests

Tests live in `testsrc/OpenStreetMapTest.cpp`. Run with:

```bash
make directories && make run_osmtest
# or after a full build:
./testbin/testosm
```

| Test | What it covers |
|------|---------------|
| `OSMTest.SimpleFiles` | `NodeCount`=2, `WayCount`=1, `NodeByIndex` bounds, `WayByIndex` bounds, `NodeByID`, `ID()`, `Location()` |
