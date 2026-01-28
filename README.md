# Supported SQL Commands:

- `CREATE UNTYPED TABLE` with column modifiers: `PRIMARY KEY`, `DEFAULT` , `NOT NULL`
- `DESCRIBE table_name`
- `DROP table_name`
- `INSERT` with or without an optional column list, and with either a single value record or multiple value records. Supports implicit and explicit empty literals and default value substitutions
- `DELETE FROM table_name [WHERE condition]`, with an optimized execution path for the specialized case when "condition" is a primary key equality. For other cases, it performs a sequential file scan. In all cases, deletion consists of marking records as TOMBSTONES, and only performing a physical deletion when the tombstone / live record ratio exceeds a certain threshold
- `UPDATE table_name SET col_name = literal_value [, col_name_2 = literal_2, ...] [WHERE condition]`. 
UPDATE is implemented in several steps:
  1. Deserialize the old records and mark them as tombstones
  2. Construct the new records by performing an in-memory projection based on an inverse projection mask (reuse old values where no update is needed, and use new values where they were given)
  3. Append the new records to the table file
  4. perform compaction if the tombstone ratio is too big
- `SELECT * FROM table_name [WHERE condition]`
- `SELECT col1, col2, col3.. FROM table_name [WHERE condition]`
- WHERE clause supports a single condition with the following comparators:
  1. `IS` / `IS NOT` --> used for NULL and TRUE / FALSE checks
  2. `LIKE` / `NOT LIKE` --> used for string pattern matching -- supports the wildcard characters `_` (to match a single arbitrary character) and `%` (to match any number of characters). 
  3. Mathematical comparators (`>, >=, <, <=`)

Type handling is the task of the user, not the task of the DB Engine. There is no strict column type checking. This means that a column can contain anything. Even a mixture of different types.

Condition evaluation however, DOES use types, by attempting to convert strings to special runtime types, based on their contents. The condition evaluator knows when to use which comparator, and gives the user errors when appropriate, with instructions.

Supported types for condition evaluation: 
- STRING (comparators: `LIKE`, `NOT LIKE`, `=`)
- BOOLEAN (comparators: `IS`, `IS NOT`)
- NULL (comparators: `IS`, `IS NOT`)
- NUMERIC (double, float, or int - comparators: `<, <=, >, >=, =`)

### **STRINGS ARE ALWAYS IN SINGLE QUOTES !!**

String handling does not support double quotes `""`. If you enter a string literal, you must strictly wrap it inside single quotes `''`

### SQL KEYWORDS ARE STRICTLY UPPERCASE!

SQL Keywords like CREATE, INSERT, DELETE, etc. are only recognized as keywords by the parser if they are entered with uppercase letters. Lowercase strings are considered identifiers by the tokenizer.

## Example commands

```
CREATE UNTYPED TABLE users (
  id PRIMARY KEY, 
  name NOT NULL, 
  age NOT NULL DEFAULT 18, 
  is_premium DEFAULT FALSE,
  badge_color DEFAULT 'purple' 
  );

DESCRIBE users;

INSERT INTO users(id, name, is_premium)
VALUES 
(1, 'George', TRUE),
(2, 'Tim', TRUE);

INSERT INTO users VALUES (3, 'Gerald', 43);

SELECT * FROM users WHERE name LIKE 'G%';

SELECT name, is_premium FROM users;

UPDATE users SET is_premium = FALSE WHERE id = 1;

UPDATE users SET name = 'Howard' WHERE age > 20;

UPDATE users SET age = 23;

DELETE FROM users WHERE id = 2;

DELETE FROM users;

SELECT * FROM users;

DROP TABLE users;

```

# Valgrind usage

## For a text report about memory usage and leaks / bad alloc errors

```

valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --verbose \
         --suppressions=../suppressions/readline.supp \
         --log-file=../valgrind-out.txt \
         ./MainApp

```

## To generate remaining suppressions while suppressing some stuff already

```

valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --verbose \
         --suppressions=../suppressions/readline.supp \
         --gen-suppressions=all \
         --log-file=../valgrind-out.txt \
         ./MainApp

```


## To generate suppressions:

```

valgrind \
  --leak-check=full \
  --show-leak-kinds=all \
  --gen-suppressions=all \
  --track-origins=yes \
  ./MainApp


```

## For heap memory usage visualization with Massif:

```
valgrind --tool=massif ./MainApp

massif-visualizer massif.out.<pid>

```

## for stack + heap tracking visualization:

```
valgrind --tool=massif --stacks=yes ./your_program
```



# Limitations:

 - **Strings cannot contain single quotes** - this is due to the implementation of the CLI driver. Without this limitation, it wouldn't be able to parse multi-line SQL input, because the line separator parsing is done in the CLI Driver and not in the tokenizer / parser. So for example, `'It's good'` wouldn't be parsed, but `'It\'s good'` would be, because in the latter example, the single quote is escaped. However, the escaping `\` character might still be written to the disk
 - **The maximum number of columns in a table is 256** (comes from the column offset map - col_id is of type `std::uint8_t`)
 - **Only one condition is allowed in the WHERE clause** - logical operators like AND, OR or BETWEEN are not supported.
 - **The maximum safe length of a record is LONG_INT_MAX bytes.** UNSIGNED_LONG_MAX number of bytes is the theoretical maximum, but due to seekg pointer manipulation and std::streamoff static casting, the record length is ultimately treated as a long int
- **the parser supports the UNIQUE column modifier, but the DB Engine doesn't** - the UNIQUE keyword is recognized and applied as a modifier, but there are no internal index structures or uniqueness checks / enforcements in the DB engine. (The PRIMARY KEY column is an exception - there are indexes and uniqueness checks for it, as you would expect)
- **No composite keys allowed** - PRIMARY KEY must be a single column
- **Primary key value cannot be updated** - the UPDATE statement will give you an error if you try to modify existing primary keys. This is to enforce better database design, because changing primary keys in real systems is a bad practice that might severely impact performance due to CASCADE and other effects, or even introduce data integrity issues.
- **This app is Linux only** - testing and development was done exclusively on a Debian 12 Linux virtual machine with 3 CPU cores and 12 GB of RAM. Compatibility with other OSs was not tested and is not guaranteed.
- **No aggregation functions in SELECT** - Aggregations like SELECT MAX, MIN or AVERAGE are not implemented
- **No aliases** - aliasing during selection is not implemented - statements like `SELECT id AS user_id ...` are not supported
- **No JOINS** - the JOIN command is not implemented. Hence, FOREIGN KEYS and CASCADE statements are also unavailable
- **No GROUP BY, ORDER BY, or HAVING COUNT clauses** - the only "extra" clause that is supported is the `WHERE` clause.
- **Memory safety** - the BISON parser is using raw pointers and is usually safe and does not cause any leaks, however, **in certain rare scenarios or syntax error paths, the parser might not dispose of the AST structure properly, and a few bytes of memory might be leaked - 80-1000 bytes, reported as "definitely" or "possibly" or "indirectly" lost by Valgrind** - based on the length of the inputs. **However, reports and visualizations generated by Massif and Valgrind report a platued memory profile**, with peak usage usually happening during initialization. This indicates that memory leaks don't occur anywhere in the app, and memory is usually reused. This is expected, because the DB Engine is using smart pointers where possible. Only the parser and the AST are using raw pointers, because they were the first components that I implemented and I deliberately used raw pointers in order to learn manual memory management and ownership principles. - **Peak memory usage with 5 different tables containing 5-10 records each, and the CLI having parsed over 15 different SQL statements was around 350 KiB.**

# Binary file structure:

table_name.bin is a single binary file that looks like this:

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

ColumnOffset is `[offset][column_id]` which is stored like this:

```
std::uint64_t offset;
std::uint8_t col_id;
```

ColumnOffset is used to speed up the evaluation of Where conditions for columns without an index 


`[payload]` is:

`[size][data][size][data]...` where each `[size][data]` pair corresponds to a single cell in the row

size is used for reading the exact number of bytes needed to get the data, data will be deserialized into an std::string

`primary_key` is `[size][data]` as well

## Offsets explained

### Offsets in the context of indexes

**In this case, the offset is absolute** - it does not include the table header - header will always be checked in the local function to ensure format consistency

This kind of offset always points to the start of the record's col_offset_region, starting from the beginning of
the record region in a table file. The record region begins right after the table file header.

### Offsets in the context of "column offset regions" in the binary file

**In this case, the offsets are always relative.** They are local to the current record in the file.
They start from the beginning of the data_tuple_region and tell us how far the actual literal value for the given column is.

For the first secondary attribute, this offset is always 0 because it is at the beginning of the data tuple region.
For the second secondary attribute this offset is sizeof(uint64_t) + first_secondary_attribute_literal.size().
And so on.

This should be used for granular deserialization in order to minimize disk IO.
This way, we don't have to deserialize entire records for condition evaluation, but instead only the single column that we need for the condition.

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

 ### Always create merge requests / pull requests. Never commit directly to `main` !!

 Except if you're the owner and know what you're doing ;)