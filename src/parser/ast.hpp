#pragma once
#include <string>

struct ASTNode {
    virtual ~ASTNode() = default;
    virtual void interpret() = 0;
};

struct CreateTableNode : public ASTNode {
    std::string tableName;
    std::string idName;

    CreateTableNode(const std::string& t, const std::string& i)
        : tableName(t), idName(i) {}

    void interpret() override;
};
