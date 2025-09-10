#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include "../parser/ast.hpp"

struct ExecutionContext
{
  std::unordered_map<std::string, std::vector<UntypedColumnDefNode *>> untyped_tables;

  ~ExecutionContext()
  {
    for (auto &it : untyped_tables)
    {
      for (auto &node : it.second)
      {
        delete node;
      }
    }
  }
};
