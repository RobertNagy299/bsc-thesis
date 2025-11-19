#pragma once
#include "../../parser/ast.hpp"
#include <unordered_map>
#include <vector>
typedef std::unordered_map<std::string, std::vector<UntypedColumnDefNode*>> untyped_table_t;

typedef struct ColModifierChecklist {
  bool has_default : 1;
  bool primary_key : 1;
  bool not_null : 1;
} colmodifiers_t;