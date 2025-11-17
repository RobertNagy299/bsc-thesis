#pragma once
#include <string>
#include <vector>
#include <iostream>

struct ProgramNode;
struct CreateUntypedTableNode;
struct UntypedColumnDefNode;
struct DropTableNode;

struct InsertNode;
struct ColumnListNode;
struct LiteralNode;
struct ValueRecordNode;
struct ValuesListNode;
struct SelectNode;

struct ComparatorNode;
struct ConditionNode;
struct ConditionListNode;
struct WhereNode;

struct DeleteNode;

struct ASTVisitor
{
  virtual void visit(ProgramNode &node) = 0;
  virtual void visit(ComparatorNode &node) = 0;
  virtual void visit(ConditionNode &node) = 0;
  virtual void visit(ConditionListNode &node) = 0;
  virtual void visit(WhereNode &node) = 0;

  virtual void visit(DeleteNode &node) = 0;

  virtual void visit(CreateUntypedTableNode &node) = 0;
  virtual void visit(UntypedColumnDefNode &node) = 0;
  virtual void visit(DropTableNode &node) = 0;
  virtual void visit(InsertNode &node) = 0;
  virtual void visit(ColumnListNode &node) = 0;
  virtual void visit(LiteralNode &node) = 0;
  virtual void visit(ValueRecordNode &node) = 0;
  virtual void visit(ValuesListNode &node) = 0;
  virtual void visit(SelectNode &node) = 0;

  virtual ~ASTVisitor() = default;
};

struct ASTNode
{
  virtual ~ASTNode() = default;
  virtual void accept(ASTVisitor &v) = 0;
};

struct ProgramNode : ASTNode
{
  std::vector<ASTNode *> statements;

  void accept(ASTVisitor &v) override
  {
    v.visit(*this);
    // delete this; -- this causes a segfault - TODO investigate
  }

  ~ProgramNode() override
  {
    std::cout << "ProgramNode destructor called!\n";
    for (auto &s : statements)
    {
      if (s)
      {
        delete s;
      }
    }
  }
};

struct CreateUntypedTableNode : ASTNode
{
  std::string tableName;
  std::vector<ASTNode *> columns;

  CreateUntypedTableNode(std::string &tableName, std::vector<ASTNode *> &columns)
      : tableName(std::move(tableName)), columns(std::move(columns)) {}

  void accept(ASTVisitor &v) override { v.visit(*this); }

  ~CreateUntypedTableNode() override
  {
    for (auto c : columns)
      if (c)
        delete c;
  }
};

struct UntypedColumnDefNode : ASTNode
{
  std::string name;
  std::vector<std::string> modifiers;

  UntypedColumnDefNode(std::string &name, std::vector<std::string> &modifiers)
      : name(std::move(name)), modifiers(std::move(modifiers))
  {
  }

  UntypedColumnDefNode(const UntypedColumnDefNode &other)
  {
    this->name = other.name;
    this->modifiers = other.modifiers;
  }

  void accept(ASTVisitor &v) override { v.visit(*this); }
};

struct DropTableNode : ASTNode
{
  std::string tableName;

  DropTableNode(std::string &tableName) : tableName(std::move(tableName)) {}

  void accept(ASTVisitor &v) override
  {
    v.visit(*this);
  }
};

// Literal values (strings, numbers, nulls, booleans, etc.)
struct LiteralNode : ASTNode
{
  enum class Type
  {
    EMPTY,
    STRING,
    NUMBER,
    NULLVAL,
    TRUEVAL,
    FALSEVAL
  };

  Type type;
  std::string value; // e.g. "123", "John Doe" (for numbers too, store as string first)

  LiteralNode(Type t, const std::string &val) : type(t), value(val) {}

  void accept(ASTVisitor &v) override { v.visit(*this); };
};

// A list of literal values = one row/record in VALUES
struct ValueRecordNode : ASTNode
{
  std::vector<LiteralNode *> values;

  ValueRecordNode(std::vector<LiteralNode *> vals) : values(std::move(vals)) {}

  ~ValueRecordNode()
  {
    for (auto v : values)
      delete v;
  }

  void accept(ASTVisitor &v) override { v.visit(*this); };
};

// The full VALUES (...) , (...) , (...) part
struct ValuesListNode : ASTNode
{
  std::vector<ValueRecordNode *> records;

  ValuesListNode(std::vector<ValueRecordNode *> recs) : records(std::move(recs)) {}

  ~ValuesListNode()
  {
    for (auto r : records)
      delete r;
  }

  void accept(ASTVisitor &v) override { v.visit(*this); };
};

// Column list: (id, name, age)
struct ColumnListNode : ASTNode
{
  std::vector<std::string> columns;

  ColumnListNode(std::vector<std::string> cols) : columns(std::move(cols)) {}

  void accept(ASTVisitor &v) override { v.visit(*this); };
};

/**
 * @brief Class used to represent an INSERT statement's Abstract Syntax Tree Node
 *
 *
 * @param tableName std::string
 * @param columns ColumnListNode*
 * @param values ValuesListNode*
 */
struct InsertNode : ASTNode
{
  std::string tableName;
  ColumnListNode *columns; // optional (nullptr if not provided)
  ValuesListNode *values;  // must exist

  InsertNode(const std::string &t, ColumnListNode *c, ValuesListNode *v)
      : tableName(t), columns(c), values(v) {}

  ~InsertNode()
  {
    delete columns;
    delete values;
  }

  void accept(ASTVisitor &v) override { v.visit(*this); };
};

struct ComparatorNode : ASTNode
{

  enum class Type
  {
    IS,
    IS_NOT,
    LIKE,
    NOT_LIKE,
    EQ,
    NE,
    LT,
    LE,
    GT,
    GE,
  };

  Type type;

  ComparatorNode(Type type) : type(type) {}

  void accept(ASTVisitor &v) override { v.visit(*this); };
};

struct ConditionNode : ASTNode
{
  std::string col_name;
  ComparatorNode *cmp_node;
  LiteralNode *literal_value;

  ConditionNode(const std::string &s, ComparatorNode *n, LiteralNode *l)
      : col_name(s), cmp_node(n), literal_value(l) {}

  void accept(ASTVisitor &v) override { v.visit(*this); };

  ~ConditionNode()
  {
    delete cmp_node;
    delete literal_value;
  }
};

struct ConditionListNode : ASTNode
{
  std::vector<ConditionNode *> conditions;

  ConditionListNode(std::vector<ConditionNode *> v)
      : conditions(std::move(v)) {}

  void accept(ASTVisitor &v) override { v.visit(*this); };

  ~ConditionListNode()
  {
    for (auto cond : conditions)
    {
      delete cond;
    }
  }
};

struct WhereNode : ASTNode
{

  ConditionListNode *conditions_list_node;

  WhereNode(ConditionListNode *cndl) : conditions_list_node(cndl) {}

  void accept(ASTVisitor &v) override { v.visit(*this); };

  ~WhereNode()
  {
    delete conditions_list_node;
  }
};

struct SelectNode : ASTNode
{
  ColumnListNode *columns; // optional (nullptr if not provided)
  std::string tableName;
  WhereNode *opt_where_node; // nullptr if not provided

  SelectNode(ColumnListNode *c, std::string &t, WhereNode *wheren)
      : tableName(t), columns(c), opt_where_node(wheren) {}

  ~SelectNode()
  {
    if (columns)
      delete columns;

    if (opt_where_node)
      delete opt_where_node;
  }

  void accept(ASTVisitor &v) override { v.visit(*this); };
};

struct DeleteNode : ASTNode
{
  std::string table_name;
  WhereNode *opt_where_node; // nullptr if not provided

  DeleteNode(std::string &tname, WhereNode *wnode) : table_name(tname), opt_where_node(wnode) {}

  ~DeleteNode()
  {
    if (opt_where_node)
    {
      delete opt_where_node;
    }
  }

  void accept(ASTVisitor &v) override { v.visit(*this); };
};