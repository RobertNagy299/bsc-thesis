#include "./public-api.hpp"

std::string MessageTemplateResolver::injectContext(std::string&& message_template,
                                                   const std::vector<std::string> resolved_context) {
  for (const std::string& context_value : resolved_context) {
    // Find the first occurrence of "{}"
    std::size_t pos = message_template.find("{}");

    // If a placeholder is found, replace it
    if (pos != std::string::npos) {
      message_template.replace(pos, 2, context_value); // Replace "{}" (2 chars)
    } else {
      // Optional: Handle case where replacements are left over
      break; // Stop if no more placeholders are available
    }
  }

  return std::move(message_template);
}