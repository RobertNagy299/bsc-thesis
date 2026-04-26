#pragma once
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct ProgramNode;
struct CreateUntypedTableNode;
struct UntypedColumnDefNode;
struct DropTableNode;
struct CSVImportNode;

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
struct DescribeNode;

struct AssignmentNode;
struct AssignmentListNode;
struct UpdateNode;

struct ASTVisitor {
  virtual void visit(ProgramNode& node) = 0;
  virtual void visit(ComparatorNode& node) = 0;
  virtual void visit(ConditionNode& node) = 0;
  virtual void visit(ConditionListNode& node) = 0;
  virtual void visit(WhereNode& node) = 0;
  virtual void visit(DescribeNode& node) = 0;
  virtual void visit(CSVImportNode& node) = 0;

  virtual void visit(DeleteNode& node) = 0;

  virtual void visit(AssignmentNode& node) = 0;
  virtual void visit(AssignmentListNode& node) = 0;
  virtual void visit(UpdateNode& node) = 0;

  virtual void visit(CreateUntypedTableNode& node) = 0;
  virtual void visit(UntypedColumnDefNode& node) = 0;
  virtual void visit(DropTableNode& node) = 0;
  virtual void visit(InsertNode& node) = 0;
  virtual void visit(ColumnListNode& node) = 0;
  virtual void visit(LiteralNode& node) = 0;
  virtual void visit(ValueRecordNode& node) = 0;
  virtual void visit(ValuesListNode& node) = 0;
  virtual void visit(SelectNode& node) = 0;

  virtual ~ASTVisitor() = default;
};

struct ASTNode {
  virtual ~ASTNode() = default;
  virtual void accept(ASTVisitor& v) = 0;
};

struct ProgramNode : ASTNode {
  std::vector<ASTNode*> statements;

  void accept(ASTVisitor& v) override { v.visit(*this); }

  ~ProgramNode() override {
    for (auto& s : statements) {
      if (s) {
        // clang-format off
        delete s; s = nullptr;
        // clang-format on
      }
    }
  }
};

struct CSVImportNode : ASTNode {
  std::string table_name;
  std::string file_path;

  CSVImportNode(const std::string& table_name, const std::string& file_path)
      : table_name(std::move(table_name)), file_path(std::move(file_path)) {}

  void accept(ASTVisitor& v) override { v.visit(*this); }
};

struct DescribeNode : ASTNode {
  std::string table_name;

  DescribeNode(const std::string& table_name) : table_name(std::move(table_name)) {}

  void accept(ASTVisitor& v) override { v.visit(*this); }
};

struct CreateUntypedTableNode : ASTNode {
  std::string table_name;
  std::vector<ASTNode*> columns;

  CreateUntypedTableNode(std::string& table_name, std::vector<ASTNode*>& columns)
      : table_name(std::move(table_name)), columns(std::move(columns)) {}

  void accept(ASTVisitor& v) override { v.visit(*this); }

  ~CreateUntypedTableNode() override {
    for (auto& c : columns) {
      if (c) {
        // clang-format off
        delete c; c = nullptr;
        // clang-format on
      }
    }
  }
};

struct UntypedColumnDefNode : ASTNode {
  std::string name;
  std::vector<std::string> modifiers;

  UntypedColumnDefNode(std::string& name, std::vector<std::string>& modifiers)
      : name(std::move(name)), modifiers(std::move(modifiers)) {}

  UntypedColumnDefNode(const UntypedColumnDefNode& other) {
    this->name = other.name;
    this->modifiers = other.modifiers;
  }

  void accept(ASTVisitor& v) override { v.visit(*this); }
};

struct DropTableNode : ASTNode {
  std::string table_name;

  DropTableNode(std::string& table_name) : table_name(std::move(table_name)) {}

  void accept(ASTVisitor& v) override { v.visit(*this); }
};

// Literal values (strings, numbers, nulls, booleans, etc.)
struct LiteralNode : ASTNode {
  enum class Type { EMPTY, STRING, NUMBER, NULLVAL, TRUEVAL, FALSEVAL };

  Type type;
  std::string value; // e.g. "123", "John Doe" (for numbers too, store as string first)

  LiteralNode(Type t, const std::string& val) : type(t), value(val) {}

  void accept(ASTVisitor& v) override { v.visit(*this); };
};

// A list of literal values = one row/record in VALUES
struct ValueRecordNode : ASTNode {
  std::vector<LiteralNode*> values;

  ValueRecordNode(std::vector<std::string> csv_row) {
    for (const auto& literal : csv_row) {
      auto type = LiteralNode::Type::STRING;
      this->values.push_back(new LiteralNode(type, literal));
    }
  }

  ValueRecordNode(std::vector<LiteralNode*> vals) : values(std::move(vals)) {}
  ValueRecordNode(std::vector<std::unique_ptr<LiteralNode>> vals) {
    for (auto& smart_ptr : vals) {
      // transfer ownership
      values.push_back(std::move(smart_ptr.release()));
    }
  }

  ~ValueRecordNode() {
    for (auto& v : values) {
      if (v) {
        // clang-format off
        delete v; v = nullptr;
        // clang-format on
      }
    }
  }

  void accept(ASTVisitor& v) override { v.visit(*this); };
};

// The full VALUES (...) , (...) , (...) part
struct ValuesListNode : ASTNode {
  std::vector<ValueRecordNode*> records;

  ValuesListNode(std::vector<ValueRecordNode*> recs) : records(std::move(recs)) {}

  ~ValuesListNode() {
    for (auto& r : records) {
      if (r) {
        // clang-format off
        delete r; r = nullptr;
        // clang-format on
      }
    }
  }

  void accept(ASTVisitor& v) override { v.visit(*this); };
};

// Column list: (id, name, age)
struct ColumnListNode : ASTNode {
  std::vector<std::string> columns;

  ColumnListNode(std::vector<std::string> cols) : columns(std::move(cols)) {}

  void accept(ASTVisitor& v) override { v.visit(*this); };
};

/**
 * @brief Class used to represent an INSERT statement's Abstract Syntax Tree Node
 *
 *
 * @param table_name std::string
 * @param columns ColumnListNode*
 * @param values ValuesListNode*
 */
struct InsertNode : ASTNode {
  std::string table_name;
  ColumnListNode* columns; // optional (nullptr if not provided) - full after normalization
  ValuesListNode* values;  // must exist

  std::vector<bool> projection_mask; // same length as schema
  bool is_normalized = false;

  InsertNode(const std::string& t, ColumnListNode* c, ValuesListNode* v) : table_name(t), columns(c), values(v) {}

  ~InsertNode() {
    if (columns) {
      // clang-format off
      delete columns; columns = nullptr;
      // clang-format on
    }
    if (values) {
      // clang-format off
      delete values; values = nullptr;
      // clang-format on
    }
  }

  void accept(ASTVisitor& v) override { v.visit(*this); };
};

struct ComparatorNode : ASTNode {

  enum class Type {
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

  void accept(ASTVisitor& v) override { v.visit(*this); };
};

struct ConditionNode : ASTNode {
  std::string col_name;
  ComparatorNode* cmp_node;
  LiteralNode* literal_value;

  std::uint8_t schema_index = 255u; // resolved column index

  ConditionNode(const std::string& s, ComparatorNode* n, LiteralNode* l) : col_name(s), cmp_node(n), literal_value(l) {}

  void accept(ASTVisitor& v) override { v.visit(*this); };

  ~ConditionNode() {
    if (cmp_node) {
      // clang-format off
      delete cmp_node; cmp_node = nullptr;
      // clang-format on
    }
    if (literal_value) {
      // clang-format off
      delete literal_value; literal_value = nullptr;
      // clang-format on
    }
  }
};

struct ConditionListNode : ASTNode {
  std::vector<ConditionNode*> conditions;

  ConditionListNode(std::vector<ConditionNode*> v) : conditions(std::move(v)) {}

  void accept(ASTVisitor& v) override { v.visit(*this); };

  ~ConditionListNode() {
    for (auto& cond : conditions) {
      if (cond) {
        // clang-format off
        delete cond; cond = nullptr;
        // clang-format on
      }
    }
  }
};

struct WhereNode : ASTNode {

  ConditionListNode* conditions_list_node; // only contains one conditionNode for now

  WhereNode(ConditionListNode* cndl) : conditions_list_node(cndl) {}

  void accept(ASTVisitor& v) override { v.visit(*this); };

  ~WhereNode() {
    if (conditions_list_node) {
      // clang-format off
      delete conditions_list_node; conditions_list_node = nullptr;
      // clang-format on
    }
  }
};

struct SelectNode : ASTNode {
  ColumnListNode* columns; // optional (nullptr if not provided)
  std::string table_name;
  WhereNode* opt_where_node; // nullptr if not provided

  std::vector<bool> projection_mask; // same size as schema
  bool is_normalized = false;

  SelectNode(ColumnListNode* c, std::string& t, WhereNode* wheren)
      : table_name(t), columns(c), opt_where_node(wheren) {}

  ~SelectNode() {
    if (columns) {
      // clang-format off
      delete columns; columns = nullptr;
      // clang-format on
    }

    if (opt_where_node) {
      // clang-format off
      delete opt_where_node; opt_where_node = nullptr;
      // clang-format on
    }
  }

  void accept(ASTVisitor& v) override { v.visit(*this); };
};

struct DeleteNode : ASTNode {
  std::string table_name;
  WhereNode* opt_where_node; // nullptr if not provided

  DeleteNode(std::string& tname, WhereNode* wnode) : table_name(tname), opt_where_node(wnode) {}

  ~DeleteNode() {
    if (opt_where_node) {
      // clang-format off
      delete opt_where_node; opt_where_node = nullptr;
      // clang-format on
    }
  }

  void accept(ASTVisitor& v) override { v.visit(*this); };
};

struct AssignmentNode : ASTNode {
  std::string col_name;
  LiteralNode* literal_node;

  AssignmentNode(std::string& cname, LiteralNode* lnode) : col_name(cname), literal_node(lnode) {}

  ~AssignmentNode() {
    if (literal_node) {
      // clang-format off
      delete literal_node; literal_node = nullptr;
      // clang-format on
    }
  }

  void accept(ASTVisitor& v) override { v.visit(*this); };
};

struct AssignmentListNode : ASTNode {
  std::vector<AssignmentNode*> assignments;

  AssignmentListNode(std::vector<AssignmentNode*> av) : assignments(std::move(av)) {}

  ~AssignmentListNode() {
    for (auto& node_p : assignments) {
      if (node_p) {
        // clang-format off
        delete node_p; node_p = nullptr;
        // clang-format on
      }
    }
  }

  void accept(ASTVisitor& v) override { v.visit(*this); };
};

struct UpdateNode : ASTNode {
  std::string table_name;
  AssignmentListNode* assignment_list_node;
  WhereNode* opt_where_node; // optional, nullptr if not provided

  std::vector<bool> inverse_proj_mask; // stores which values keep their old form
  bool is_normalized = false;
  std::unordered_map<std::size_t, std::string> schema_index_to_literal_map;

  UpdateNode(std::string& tn, AssignmentListNode* aln_p, WhereNode* wn_p)
      : table_name(tn), assignment_list_node(aln_p), opt_where_node(wn_p) {}

  ~UpdateNode() {
    if (assignment_list_node) {
      // clang-format off
      delete assignment_list_node; assignment_list_node = nullptr;
      // clang-format on
    }
    if (opt_where_node) {
      // clang-format off
      delete opt_where_node; opt_where_node = nullptr;
      // clang-format on
    }
  }

  void accept(ASTVisitor& v) override { v.visit(*this); };
};