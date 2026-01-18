#include "../utils_public_api.hpp"
#include <string>
#include <vector>

// Function to trim leading and trailing whitespace
std::string Utilities::StringUtils::trim(const std::string& str) {
  const std::string whitespace = " \t\n\r\f\v"; // Common whitespace characters

  // Find the first non-whitespace character
  std::size_t first_non_ws = str.find_first_not_of(whitespace);
  if (std::string::npos == first_non_ws) {
    return ""; // Entire string is whitespace
  }

  // Find the last non-whitespace character
  std::size_t last_non_ws = str.find_last_not_of(whitespace);

  // Extract the substring
  return str.substr(first_non_ws, (last_non_ws - first_non_ws + 1));
}

// Splits an input string based on the given delimiter string, and returns a vector of substrings.
std::vector<std::string> Utilities::StringUtils::splitString(const std::string& s, const std::string& delimiter) {
  std::vector<std::string> tokens;
  std::size_t start = 0;
  std::size_t end = s.find(delimiter);

  while (end != std::string::npos) {
    tokens.push_back(s.substr(start, end - start));
    start = end + delimiter.length();
    end = s.find(delimiter, start);
  }
  tokens.push_back(s.substr(start)); // Add the last token

  return tokens;
}

std::string Utilities::StringUtils::removeOuterQuotes(std::string str) {
  if (str.length() >= 2) {
    // Check for double quotes
    if (str.front() == '"' && str.back() == '"') {
      // Remove the first and the last character
      return str.substr(1, str.length() - 2);
    }
    // Check for single quotes
    if (str.front() == '\'' && str.back() == '\'') {
      // Remove the first and the last character
      return str.substr(1, str.length() - 2);
    }
  }
  // Return the original string if no matching quotes were found
  return str;
}

/**
 * Checks if the prefix of a string matches a given prefix pattern.
 *
 * @param mainString The string to check the prefix of.
 * @param prefixPattern The pattern to match against the start of mainString.
 * @return true if mainString starts with prefixPattern, false otherwise.
 */
bool Utilities::StringUtils::startsWith(const std::string& mainString, const std::string& prefixPattern) {
  // rfind checks for the last occurrence of prefixPattern within mainString
  // and checks if that last occurrence is at position 0 (the beginning).
  return mainString.rfind(prefixPattern, 0) == 0;
}

/**
 * @brief Checks if a string ends with a specific suffix pattern.
 *
 * @param str The main string to check.
 * @param suffix The suffix pattern to look for.
 * @return true if str ends with suffix, false otherwise.
 */
bool Utilities::StringUtils::hasSuffix(const std::string& str, const std::string& suffix) {
  if (str.length() >= suffix.length()) {
    // Check if the substring of 'str' starting from the appropriate index
    // matches the 'suffix'.
    return (str.rfind(suffix) == (str.length() - suffix.length()));
  } else {
    return false;
  }
}