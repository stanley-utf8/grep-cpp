#include <cctype>
#include <cerrno>
#include <cstddef>
#include <ios>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Helper method to split a string by '|'
std::vector<std::string> split(const std::string &str) {
  std::vector<std::string> tokens;
  std::stringstream ss(str);
  std::string token;

  while (std::getline(ss, token, '|')) {
    tokens.push_back(token);
  }
  return tokens;
}

// Tokenize the pattern into tokens, now with group_counter to track group
// numbers
std::vector<std::string> tokenize_pattern(std::string &pattern,
                                          int &group_counter) {
  std::vector<std::string> tokens;

  std::cout << "input pattern: '" << pattern << "'\n";

  for (size_t i = 0; i < pattern.length(); i++) {
    std::cout << "\nProcessing index: " << i << ", char: '" << pattern[i]
              << "'\n";
    if (pattern[i] == '\\') {
      std::cout << "'\\' found at index: " << i << "\n";
      if (i + 1 < pattern.length()) {
        std::string escaped_token = pattern.substr(i, 2);
        // Check for quantifier after escaped token
        size_t next_pos = i + 2;
        if (next_pos < pattern.length()) {
          char next_char = pattern[next_pos];
          if (next_char == '+' || next_char == '?') {
            escaped_token += next_char;
            i = next_pos; // Move past the quantifier
          }
        }
        std::cout << "Adding escaped token: '" << escaped_token << "'\n";
        tokens.push_back(escaped_token);
        i++; // Skip the next character as it's part of the escaped token or
             // quantifier
      } else {
        std::cout << "WARNING: escape character at end of pattern\n";
      }
    } else if (pattern[i] == '[') {
      std::cout << "'[' found at index: " << i << "\n";
      size_t end = pattern.find(']', i);
      if (end == std::string::npos) {
        throw std::runtime_error("Matching ']' not found in: " + pattern);
      }
      std::string token = pattern.substr(i, end - i + 1);
      // Check for quantifier after ]
      size_t next_pos = end + 1;
      if (next_pos < pattern.length()) {
        char next_char = pattern[next_pos];
        if (next_char == '+' || next_char == '?') {
          token += next_char;
          end = next_pos; // Move past the quantifier
        }
      }
      std::cout << "Adding character class token: '" << token
                << "', end index: " << end << "\n";
      tokens.push_back(token);
      i = end; // Move to the end of the token (including quantifier if any)
    } else if (pattern[i] == '(') {
      std::cout << "'(' found at index: " << i << "\n";
      size_t end = pattern.find(')', i);
      if (end == std::string::npos) {
        throw std::runtime_error("Matching ')' not found in: " + pattern);
      }
      group_counter++;
      std::string content = pattern.substr(i + 1, end - i - 1);
      std::string token =
          "(" + std::to_string(group_counter) + ":" + content + ")";
      // Check for quantifier after )
      size_t next_pos = end + 1;
      if (next_pos < pattern.length()) {
        char next_char = pattern[next_pos];
        if (next_char == '+' || next_char == '?') {
          token += next_char;
          end = next_pos; // Move past the quantifier
        }
      }
      std::cout << "Adding group token: '" << token << "', end index: " << end
                << "\n";
      tokens.push_back(token);
      i = end; // Move to the end of the token (including quantifier if any)
    } else {
      std::string single_char(1, pattern[i]);
      if (i + 1 < pattern.size() &&
          (pattern[i + 1] == '+' || pattern[i + 1] == '?')) {
        std::string one_more = pattern.substr(i, 2);
        std::cout << "Adding '" << one_more << "'\n";
        tokens.push_back(one_more);
        i++; // Skip the quantifier as it's part of the token
      } else {
        std::cout << "Adding single character token: '" << single_char << "'\n";
        tokens.push_back(single_char);
      }
    }
  }
  std::cout << "\nFinal tokens:\n";
  for (size_t i = 0; i < tokens.size(); i++) {
    std::cout << i << ": '" << tokens[i] << "'\n";
  }
  return tokens;
}

// Helper to match a character against a character group token (like "[a-z]")
bool match_char_group(char c, const std::string &pattern) {
  if (c == '\0') {
    return false;
  }
  bool is_negative = (pattern.length() > 1 and pattern[1] == '^');
  int idx = is_negative ? 2 : 1;

  std::unordered_set<char> char_set;
  while (idx < pattern.length() - 1) {
    char cur = pattern[idx];
    char next = pattern[idx + 1];

    if (next == '-') {
      if (idx + 2 >= pattern.length() - 1) {
        char_set.insert(cur);
        char_set.insert(next);
        idx += 2;
      } else {
        char start = cur;
        char end = pattern[idx + 2];
        std::cerr << "Adding range from " << start << " to " << end << "\n";
        for (char ch = start; ch <= end; ch++) {
          char_set.insert(ch);
        }
        idx += 3;
      }
    } else {
      char_set.insert(cur);
      idx += 1;
    }
  }
  std::cerr << "current char set:";
  for (char ch : char_set) {
    std::cerr << ch << " ";
  }
  std::cerr << "\n";

  if (is_negative) {
    return char_set.find(c) == char_set.end();
  } else {
    return char_set.find(c) != char_set.end();
  }
}

// Match a single token at a given position in the input.
bool match_token(size_t idx, const std::string &input_line,
                 const std::string &token) {
  if (idx >= input_line.size())
    return false;
  char c = input_line[idx];

  if (token == "\\d") {
    return isdigit(c);
  }
  if (token == "$") {
    return idx == input_line.size();
  }
  if (token == ".") {
    return true;
  }
  if (token == "\\w") {
    return isalpha(c);
  }
  if (token.length() == 1) {
    return token[0] == c;
  }
  if (token[0] == '[') {
    return match_char_group(c, token.substr(0, token.find_first_of("+?")));
  }
  throw std::runtime_error("Unhandled token: " + token);
  return false;
}

// Handle quantifiers for a token (common logic for + and ?)
bool handle_quantifier(const std::string &token, size_t &pos,
                       const std::string &input_line, bool is_plus) {
  if (token.empty())
    return false;
  char quantifier = token.back();
  std::string base_token = token.substr(0, token.size() - 1);

  if (quantifier == '+') {
    size_t count = 0;
    while (pos < input_line.size() &&
           match_token(pos, input_line, base_token)) {
      pos++;
      count++;
    }
    return count >= 1;
  } else if (quantifier == '?') {
    if (pos < input_line.size() && match_token(pos, input_line, base_token)) {
      pos++;
    }
    return true;
  }
  return false;
}

/*
  Updated match_from function to handle quantifiers for character classes and
  other tokens.
*/
bool match_from(const std::vector<std::string> &tokens,
                const std::string &input_line, size_t &pos,
                std::unordered_map<std::string, std::string> &groups,
                int &group_counter) {
  for (size_t i = 0; i < tokens.size(); i++) {
    const std::string &token = tokens[i];
    std::cout << "At pos " << pos << ", matching token: '" << token << "'\n";

    if (token.empty())
      continue;

    // Handle start (^) and end ($) anchors.
    if (token == "^") {
      if (pos != 0)
        return false;
    } else if (token == "$") {
      if (pos != input_line.size())
        return false;
    }
    // If token is a capturing group, parse group number and content.
    else if (token[0] == '(') {
      size_t colon_pos = token.find(':');
      if (colon_pos == std::string::npos || token.back() != ')') {
        throw std::runtime_error("Invalid group token: " + token);
      }
      std::string group_num_str = token.substr(1, colon_pos - 1);
      int group_num = std::stoi(group_num_str);
      std::string group_content = token.substr(
          colon_pos + 1,
          token.size() - colon_pos - 2 -
              (token.back() == '+' || token.back() == '?' ? 1 : 0));

      size_t groupStart = pos;
      std::vector<std::string> alternatives = split(group_content);
      bool groupMatched = false;
      size_t savedPos = pos;
      for (auto &alt : alternatives) {
        pos = savedPos;
        std::vector<std::string> groupTokens =
            tokenize_pattern(alt, group_counter);
        if (match_from(groupTokens, input_line, pos, groups, group_counter)) {
          groupMatched = true;
          break;
        }
      }
      if (!groupMatched)
        return false;

      std::string groupMatch = input_line.substr(groupStart, pos - groupStart);
      groups[group_num_str] = groupMatch;
      std::cout << "Captured group " << group_num << ": " << groupMatch << "\n";
    }
    // Handle backreferences like \1, \2 etc.
    else if (token.size() >= 2 && token[0] == '\\' && isdigit(token[1])) {
      std::string group_num_str = token.substr(1);
      auto it = groups.find(group_num_str);
      if (it == groups.end()) {
        std::cout << "Backreference to non-existent group: " << group_num_str
                  << "\n";
        return false;
      }
      std::string expected = it->second;
      std::cout << "Matching backreference to group " << group_num_str
                << ", expecting: " << expected << "\n";
      if (input_line.substr(pos, expected.length()) != expected) {
        return false;
      }
      pos += expected.length();
    }
    // Handle tokens with quantifiers
    else if (token.back() == '+' || token.back() == '?') {
      if (!handle_quantifier(token, pos, input_line, token.back() == '+'))
        return false;
    }
    // Otherwise, match a normal token (assumed to consume one character).
    else {
      if (!match_token(pos, input_line, token))
        return false;
      pos++;
    }
  }
  return true;
}

/*
  Top-level match_pattern.
*/
bool match_pattern(const std::string &input_line,
                   const std::vector<std::string> &tokens, int &group_counter) {
  if (tokens.empty()) {
    return input_line.empty();
  }

  bool anchored_start = (tokens[0] == "^");
  if (anchored_start) {
    size_t pos = 0;
    std::unordered_map<std::string, std::string> groups;
    return match_from(tokens, input_line, pos, groups, group_counter);
  } else {
    for (size_t start_pos = 0; start_pos <= input_line.size(); ++start_pos) {
      size_t pos = start_pos;
      std::unordered_map<std::string, std::string> groups;
      if (match_from(tokens, input_line, pos, groups, group_counter)) {
        return true;
      }
    }
    return false;
  }
}

int main(int argc, char *argv[]) {
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  if (argc != 3) {
    std::cerr << "Expected two arguments" << std::endl;
    return 1;
  }

  std::string flag = argv[1];
  std::string pattern = argv[2];

  if (flag != "-E") {
    std::cerr << "Expected first argument to be '-E'" << std::endl;
    return 1;
  }

  std::string input_line;
  std::getline(std::cin, input_line);

  try {
    int group_counter = 0;
    std::vector<std::string> tokens = tokenize_pattern(pattern, group_counter);
    if (match_pattern(input_line, tokens, group_counter))
      return 0;
    else
      return 1;
  } catch (const std::runtime_error &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
}
