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

struct ASTVisitor
{
  virtual void visit(ProgramNode &node) = 0;
  virtual void visit(CreateUntypedTableNode &node) = 0;
  virtual void visit(UntypedColumnDefNode &node) = 0;
  virtual void visit(DropTableNode &node) = 0;
  virtual void visit(InsertNode &node) = 0;
  virtual void visit(ColumnListNode &node) = 0;
  virtual void visit(LiteralNode &node) = 0;
  virtual void visit(ValueRecordNode &node) = 0;
  virtual void visit(ValuesListNode &node) = 0;

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

// INSERT statement itself
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
