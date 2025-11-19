#include "../public_api.hpp"

ExecutionContext::~ExecutionContext() {
  for (auto& it : untyped_tables) {
    for (UntypedColumnDefNode*& node : it.second) {
      if (node) {
        // clang-format off
          delete node; node = nullptr;
        // clang-format on
      }
    }
  }
}