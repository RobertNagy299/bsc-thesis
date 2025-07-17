#include <iostream>

#include "parser/parser.h"

int main() {
    std::cout << "hi!\n" << __cplusplus << '\n';
    auto myParser = SQLParser();
    myParser.initParser();

    return 0;
}