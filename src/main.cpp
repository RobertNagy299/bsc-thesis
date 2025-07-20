#include <iostream>

#include "parser/parser.h"
#include "DBEngine/engine.h"

int main() {
    std::cout << "hi!\n" << __cplusplus << '\n';
    auto myParser = SQLParser();
    auto Engine = DataBaseEngine();

    Engine.initDBEngine();
    myParser.initParser();

    return 0;
}