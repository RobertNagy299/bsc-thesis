#include <gtest/gtest.h>
#include "../src/parser/ast.hpp"

// Forward declare parser API
extern int yyparse();
extern ASTNode *root;
extern FILE *yyin;
extern int yylex_destroy();


TEST(ParserTest, CreateTableCorrectly)
{
  const char *sql = "CREATE TABLE users (id PRIMARY KEY);";

  // Open SQL string as input
  FILE *f = fmemopen((void *)sql, strlen(sql), "r");
  ASSERT_NE(f, nullptr);

  yyin = f;
  root = nullptr;

  int result = yyparse();
  yylex_destroy();
  fclose(f);

  ASSERT_EQ(result, 0);
  ASSERT_NE(root, nullptr);

  // Cleanup
  delete root;
  root = nullptr;
}


TEST(ParserTest, CreateTableWithoutTableKeyword)
{
  const char *sql = "CREATE users;";

  // Open SQL string as input
  FILE *f = fmemopen((void *)sql, strlen(sql), "r");
  ASSERT_NE(f, nullptr);

  yyin = f;
  root = nullptr;

  int result = yyparse();
  yylex_destroy();
  fclose(f);

  ASSERT_NE(result, 0);
  ASSERT_EQ(root, nullptr);

  // Cleanup
  delete root;
  root = nullptr;
}