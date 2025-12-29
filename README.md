# Valgrind usage

```

valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --verbose \
         --log-file=../valgrind-out.txt \
         ./MainApp

```

# Commands:

[optional]

DROP TABLE identifier; \
INSERT INTO table[(col1, col2)] VALUES (asd, asd1) [, (asd3, asd4)]; \
CREATE UNTYPED TABLE table (col1 [PRIMARY KEY] [, col2 [NOT NULL], col3 [UNIQUE], col4 [DEFAULT 'str1']]); \
SELECT (col1, col2) FROM table \
SELECT * FROM table \

# Limitations:

 - Maximum # of columns in a table: 256 (comes from the column offset map - col_id is of type uint8_t)
 - Only one condition is allowed in the WHERE clause

# Binary file structure:

Table.dat is a single binary file that looks like this:

`[table header][record][record][record]...`

Where `[table header]` is 32 bytes and looks like this:
`[magic][version][flags][reserved]`
``` 
uint64_t magic; // unique file identifier
uint64_t version;
uint64_t flags;
uint64_t reserved;
```

and [record] looks like this:

`[record_len][record_type][col_offset_map][primary_key][payload]`

```
uint64_t record_len;
RecordType record_type; // sizeof(uint8_t)
column_offset_t column_offset_map; // (# of cols) * sizeof(ColumnOffset) 
primary_key_t primary_key; // variable length
std::string payload; // variable length
```

ColumnOffset is `[column_id][offset]` which is stored like this:

```
uint8_t col_id;
uint64_t offset;
```

ColumnOffset is used to speed up the evaluation of Where conditions for columns without an index 

`[payload]` is:

`[size][data][size][data]...` where each `[size][data]` pair corresponds to a single cell in the row

size is used for reading the exact number of bytes needed to get the data, data will be deserialized into an std::string