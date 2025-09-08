#include <iostream>
#include "parser/ast.hpp"

extern int yyparse();
extern ASTNode* root;  // Defined in parser.y
extern FILE* yyin;
extern int yylex_destroy(); // <-- from flex

int main() {
    yyin = stdin;
    std::cout << "Enter SQL statement:\n";
    yyparse();

    // Destroy Flex's internal buffers to avoid "still reachable" memory.
    yylex_destroy();

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
