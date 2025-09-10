#pragma once
#include <string>
#include <vector>

struct ProgramNode;
struct CreateUntypedTableNode;
struct UntypedColumnDefNode;

struct ASTVisitor
{
  virtual void visit(ProgramNode &node) = 0;
  virtual void visit(CreateUntypedTableNode &node) = 0;
  virtual void visit(UntypedColumnDefNode &node) = 0;
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

  // ProgramNode(std::vector<ASTNode *> &statements)
  //     : statements(std::move(statements)) {}

  void accept(ASTVisitor &v) override { v.visit(*this); }

  ~ProgramNode()
  {
    for (auto s : statements)
      delete s;
  }
};

struct CreateUntypedTableNode : ASTNode
{
  std::string tableName;
  std::vector<ASTNode *> columns;

  CreateUntypedTableNode(std::string &tableName, std::vector<ASTNode *> &columns)
      : tableName(std::move(tableName)), columns(std::move(columns)) {}

  void accept(ASTVisitor &v) override { v.visit(*this); }

  ~CreateUntypedTableNode()
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
