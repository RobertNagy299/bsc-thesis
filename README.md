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

 - Maximum # of columns in a table: 256 (comes from the column offset map - col_id is of type std::uint8_t)
 - Only one condition is allowed in the WHERE clause
 - The maximum safe length of a record is LONG_INT_MAX bytes. UNSIGNED_LONG_MAX number of bytes is the theoretical maximum, but due to seekg pointer manipulation and std::streamoff static casting, the record length is ultimately treated as a long int

# Binary file structure:

Table.dat is a single binary file that looks like this:

`[table header][record][record][record]...`

Where `[table header]` is 32 bytes and looks like this:
`[magic][version][flags][reserved]`
``` 
std::uint64_t magic; // unique file identifier
std::uint64_t version;
std::uint64_t flags;
std::uint64_t reserved;
```

and [record] looks like this:

`[record_len][record_type][col_offset_map][primary_key][payload]`

```
std::uint64_t record_len;
DB_Types::RecordType record_type; // sizeof(std::uint8_t)
DB_Types::column_offset_t column_offset_map; // (# of cols - 1) * sizeof(ColumnOffset) 
DB_Types::primary_key_t primary_key; // variable length
std::string payload; // variable length
```

ColumnOffset is `[column_id][offset]` which is stored like this:

```
std::uint8_t col_id;
std::uint64_t offset;
```

ColumnOffset is used to speed up the evaluation of Where conditions for columns without an index 
offset is relative, tells you how far the data is from the beginning of the current record's data_tuple region (after the primary key)
`[payload]` is:

`[size][data][size][data]...` where each `[size][data]` pair corresponds to a single cell in the row

size is used for reading the exact number of bytes needed to get the data, data will be deserialized into an std::string

`primary_key` is `[size][data]` as well

# Contribution guide:

## Branch naming

If you want to contribute, you must create your own branch.
Your branch name must adhere to the following pattern:

`[project name]-[category]/[ticket number]`

where 
 - `project name` is `JSSQL`
 - `category` is either `feat` or `bug`
 - `ticket number` is the number of the issue / ticket that describes the feature or bug you're working on

Example 1:

IF there is a `bug ticket` with the number `542`
THEN your branch name is `JSSQL-bug/542`

## Commit message format:

Abstract form: `[type]([domain]): [optional_ticket_number] [your_message]`

Where
 - type is either `feat` or `bug`
 - domain is one of the following: 
   - `parser` 
   - `interpreter`
   - `code quality`
   - `file handling`
   - `optimization`
   - `memory safety`
   - `documentation`
   - or any other domains that are added to this list in the future. Try to keep the number of domains as low as possible.

Example 1:

IF a ticket or issue exists for the parser feature you're working on:
 - `example-ticket` with ticket number `1352`
THEN the commit message must be: `feat(parser): 1352 create AST node structs for the GROUP BY statement`

Example 2:
IF you are working on a memory-leak that has no ticket / issue
THEN
 - create a ticket / issue for it: `memory-leak-issue asdfgh`, ticket number = `123`
 - add a description to the ticket - prefer verbosity and precision over ambiguity
THEN create a commit like this:
 - `bug(memory safety): 123 Add proper bison destructor for GROUP BY node in case of a syntax error`