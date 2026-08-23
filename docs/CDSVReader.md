# CDSVReader

Reads DSV (delimiter-separated values) data from a `CDataSource`. Supports various delimiting characters and uses standard CSV-style escaping. Handles quoted fields and escaped quotes inside quoted fields. Reads input incrementally from a CDataSource with no unnecessary buffering of entire files.

## Constructor

`CDSVReader(std::shared_ptr<CDataSource> src, char delimiter)`

**Parameters:**
- `src` - Data source that provides character-level access to input
- `delimiter` - Character delimiter used to separate fields

## Destructor

`~CDSVReader()` - Destroys the reader.

## Methods

### bool End() const

Returns `true` if all rows have been read from the DSV data source, `false` otherwise.

### bool ReadRow(std::vector<std::string> &row)

Reads a single row from the data source. The row vector is cleared before being populated with the parsed fields. Each field is appended as a string. Reading stops at a newline or end of input.

**Parameters:**
- `row` - Output vector that will contain the fields from the row (one string per column)

**Returns:** `true` if a row was successfully read, `false` if no more rows are available.

## Example

```cpp
auto src = std::make_shared<CStringDataSource>("a,b,c\nd,e,f");
CDSVReader reader(src, ','); 
std::vector<std::string> row; 

while (reader.ReadRow(row))
{
    for (const auto &field : row)
    {
        std::cout << field << "|"; 
    }
    std::cout << std::endl; 
}
```

