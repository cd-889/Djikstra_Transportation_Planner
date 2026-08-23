# CDSVWriter

Writes DSV (delimiter-separated values) data to a `CDataSink`. Every call to `WriteRow` outputs one row with fields separated by a configurable delimiter.

## Constructor

`CDSVWriter(std::shared_ptr<CDataSink> sink, char delimiter, bool quoteall = false)`

**Parameters:**
- `sink` - Destination data sink receiving written output
- `delimiter` - Character delimiter used to separate fields
- `quoteall` - If true, all fields are quoted; if false, only fields containing the delimiter, quotes, or newlines are quoted

## Destructor

`~CDSVWriter()` - Destroys the writer.

## Methods

### bool WriteRow(const std::vector<std::string> &row)

Writes a single row of data to the data sink. Each string in the vector represents one field and is written in order, with the delimiter character inserted between fields. Fields containing the delimiter, double quotes, or newlines are automatically quoted. Double quotes within fields are escaped as two consecutive double quotes.

**Returns:** `true` if the row is successfully written.

## Example

```cpp
auto sink = std::make_shared<CStringDataSink>();
CDSVWriter writer(sink, ',', false); 
writer.WriteRow({"1", "2", "3"}); 
writer.WriteRow({"4", "5", "6"});

// Output: "1,2,3\n4,5,6\n"
```

