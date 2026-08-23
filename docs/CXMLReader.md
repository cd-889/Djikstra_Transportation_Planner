# CXMLReader

Built on top of the Expat XML parser, this class reads XML data from a `CDataSource` and converts parsing events into discrete `SXMLEntity` objects.

## Constructor

`CXMLReader(std::shared_ptr<CDataSource> src)`

**Parameters:**
- `src` - XML input source providing raw character data

## Destructor

`~CXMLReader()` - Destroys the reader. Parser cleanup is handled internally.

## Methods

### bool End() const

Returns `true` if the reader has reached the end of the XML stream and all entities have been read, `false` otherwise.

### bool ReadEntity(SXMLEntity &entity, bool skipcdata = false)

Reads the next XML parsing event and stores it in the entity parameter. Parses XML incrementally from the `CDataSource` and converts Expat callbacks into `SXMLEntity` objects. Entities are queued internally and returned in FIFO order, with one entity returned per call.

**Parameters:**
- `entity` - Output parameter that receives the next XML entity
- `skipcdata` - If `true`, character data entities are skipped and only element entities (StartElement, EndElement, CompleteElement) are returned

**Returns:** `true` if an entity was read successfully, `false` if no more entities are available.

## Example

```cpp
auto src = std::make_shared<CStringDataSource>("<root>Hello</root>"); 
CXMLReader reader(src); 
SXMLEntity entity;

while (reader.ReadEntity(entity))
{
    std::cout << entity.DNameData << std::endl; 
}
```


