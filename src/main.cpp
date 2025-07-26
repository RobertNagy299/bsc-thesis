#include <iostream>
#include "parser/ast.hpp"

extern int yyparse();
extern ASTNode* root;  // Defined in parser.y
extern FILE* yyin;

int main() {
    yyin = stdin;
    std::cout << "Enter SQL statement:\n";
    yyparse();
    if (root) {
        std::cout << "If is running";
        root->interpret();
        delete root;
        root = nullptr;
    } else {
        std::cerr << "Parsing failed.\n";
    }
    return 0;
}
