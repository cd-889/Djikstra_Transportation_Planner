# CXMLWriter

Writes XML entities to a `CDataSink`. Supports all XML entity types: StartElement, EndElement, CharData, and CompleteElement. Automatically escapes special XML characters (`&` `<` `>` `"` `'`) in attributes and character data. Tracks open elements in a stack to ensure proper nesting.

## Constructor

`CXMLWriter(std::shared_ptr<CDataSink> sink)`

**Parameters:**
- `sink` - Destination data sink for XML output

## Destructor

`~CXMLWriter()` - Automatically calls `Flush()` to close any remaining open elements.

## Methods

### bool Flush()

Writes closing tags (`</name>`) for all currently open elements in reverse order (most recently opened first). Should be called after writing all entities to ensure properly closed XML.

**Returns:** `true` on success.

### bool WriteEntity(const SXMLEntity &entity)

Writes a single XML entity to the output sink. The behavior depends on the entity type:

- **StartElement** → Outputs `<name attr="val">` (pushes onto stack)
- **EndElement** → Outputs `</name>` (pops from stack)
- **CharData** → Outputs escaped text content
- **CompleteElement** → Outputs self-closing tag `<name attr="val"/>`

**Parameters:**
- `entity` - The XML entity to write (set `DType`, `DNameData`, and optionally `DAttributes`)

**Returns:** `true` on success.

## SXMLEntity Structure

To create an entity, set the following fields:
- `DType` - Entity type (StartElement, EndElement, CharData, or CompleteElement)
- `DNameData` - Element name (for elements) or text content (for CharData)
- `DAttributes` - Vector of name-value pairs for attributes (StartElement/CompleteElement only), or use `SetAttribute(name, value)` method

## Example

```cpp
auto sink = std::make_shared<CStringDataSink>();
CXMLWriter writer(sink);

SXMLEntity e;
e.DType = SXMLEntity::EType::StartElement;
e.DNameData = "root";
e.SetAttribute("id", "1");
writer.WriteEntity(e);

e.DType = SXMLEntity::EType::CharData;
e.DNameData = "hello";
writer.WriteEntity(e);

e.DType = SXMLEntity::EType::EndElement;
e.DNameData = "root";
writer.WriteEntity(e);

writer.Flush();
// Output: <root id="1">hello</root>
```
