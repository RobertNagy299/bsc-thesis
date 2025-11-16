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

DROP TABLE identifier;
INSERT INTO table[(col1, col2)] VALUES (asd, asd1) [, (asd3, asd4)];
CREATE UNTYPED TABLE table (col1 [PRIMARY KEY] [, col2 [NOT NULL], col3 [UNIQUE], col4 [DEFAULT 'str1']]);
SELECT (col1, col2) FROM table
SELECT * FROM table