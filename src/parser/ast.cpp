#include "ast.hpp"
#include <iostream>

void CreateTableNode::interpret() {
    std::cout << "Creating table: " << tableName
              << " with primary key: " << idName << std::endl;
    // This is where you'd call your JSON/db logic
}
