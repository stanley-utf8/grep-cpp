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

// Tokenize the pattern into tokens, now with group_counter to track group numbers
std::vector<std::string> tokenize_pattern(std::string &pattern, int &group_counter) {
  std::vector<std::string> tokens;

  for (size_t i = 0; i < pattern.length(); i++) {
    if (pattern[i] == '\\') {
      if (i + 1 < pattern.length()) {
        std::string escaped_token = pattern.substr(i, 2);
        size_t next_pos = i + 2;
        if (next_pos < pattern.length() && (pattern[next_pos] == '+' || pattern[next_pos] == '?')) {
          escaped_token += pattern[next_pos];
          i = next_pos;
        }
        tokens.push_back(escaped_token);
        i++;
      }
    } else if (pattern[i] == '[') {
      size_t end = pattern.find(']', i);
      if (end == std::string::npos) {
        throw std::runtime_error("Matching ']' not found in: " + pattern);
      }
      std::string token = pattern.substr(i, end - i + 1);
      size_t next_pos = end + 1;
      if (next_pos < pattern.length() && (pattern[next_pos] == '+' || pattern[next_pos] == '?')) {
        token += pattern[next_pos];
        end = next_pos;
      }
      tokens.push_back(token);
      i = end;
    } else if (pattern[i] == '(') {
      size_t end = pattern.find(')', i);
      if (end == std::string::npos) {
        throw std::runtime_error("Matching ')' not found in: " + pattern);
      }
      group_counter++;
      std::string content = pattern.substr(i + 1, end - i - 1);
      std::string token = "(" + std::to_string(group_counter) + ":" + content + ")";
      size_t next_pos = end + 1;
      if (next_pos < pattern.length() && (pattern[next_pos] == '+' || pattern[next_pos] == '?')) {
        token += pattern[next_pos];
        end = next_pos;
      }
      tokens.push_back(token);
      i = end;
    } else {
      std::string single_char(1, pattern[i]);
      if (i + 1 < pattern.size() && (pattern[i + 1] == '+' || pattern[i + 1] == '?')) {
        std::string one_more = pattern.substr(i, 2);
        tokens.push_back(one_more);
        i++;
      } else {
        tokens.push_back(single_char);
      }
    }
  }
  return tokens;
}

// Helper to match a character against a character group token (like "[a-z]")
bool match_char_group(char c, const std::string &pattern) {
  if (c == '\0') return false;
  bool is_negative = (pattern.length() > 1 && pattern[1] == '^');
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
        for (char ch = start; ch <= end; ch++) {
          char_set.insert(ch);
        }
        idx += 3;
      }
    } else {
      char_set.insert(cur);
      idx++;
    }
  }

  return is_negative ? (char_set.find(c) == char_set.end()) : (char_set.find(c) != char_set.end());
}

// Match a single token at a given position in the input.
bool match_token(size_t idx, const std::string &input_line, const std::string &token) {
  if (idx >= input_line.size()) return false;
  char c = input_line[idx];

  if (token == "\\d") return isdigit(c);
  if (token == "$") return idx == input_line.size();
  if (token == ".") return true;
  if (token == "\\w") return isalpha(c);
  if (token.length() == 1) return token[0] == c;
  if (token[0] == '[') return match_char_group(c, token.substr(0, token.find_first_of("+?")));
  throw std::runtime_error("Unhandled token: " + token);
}

bool match_from(const std::vector<std::string> &tokens, const std::string &input_line,
                size_t &pos, std::unordered_map<std::string, std::string> &groups, int &group_counter);

bool handle_quantifier(const std::vector<std::string> &remaining_tokens, const std::string &token,
                       size_t &pos, const std::string &input_line,
                       std::unordered_map<std::string, std::string> &groups, int &group_counter) {
  char quantifier = token.back();
  std::string base_token = token.substr(0, token.size() - 1);
  size_t initial_pos = pos;

  if (quantifier == '+') {
    // Determine maximum possible matches
    size_t max_count = 0;
    while (pos + max_count < input_line.size() && match_token(pos + max_count, input_line, base_token)) {
      max_count++;
    }
    if (max_count == 0) return false;

    // Try from max_count down to 1
    for (int count = max_count; count >= 1; --count) {
      pos = initial_pos + count;
      bool remaining_matched = match_from(remaining_tokens, input_line, pos, groups, group_counter);
      if (remaining_matched) {
        return true;
      }
      pos = initial_pos; // Reset pos for next attempt
    }
    return false;
  } else if (quantifier == '?') {
    // Try 0 or 1 occurrence
    for (int count = 1; count >= 0; --count) {
      pos = initial_pos + count;
      bool remaining_matched = (count == 0) || match_token(initial_pos, input_line, base_token);
      if (remaining_matched) {
        remaining_matched = match_from(remaining_tokens, input_line, pos, groups, group_counter);
        if (remaining_matched) return true;
      }
      pos = initial_pos; // Reset pos for next attempt
    }
    return false;
  }
  return false;
}

bool match_from(const std::vector<std::string> &tokens, const std::string &input_line,
                size_t &pos, std::unordered_map<std::string, std::string> &groups, int &group_counter) {
  if (tokens.empty()) return true;

  std::string current_token = tokens[0];
  std::vector<std::string> remaining_tokens(tokens.begin() + 1, tokens.end());

  if (current_token == "^") {
    if (pos != 0) return false;
    return match_from(remaining_tokens, input_line, pos, groups, group_counter);
  } else if (current_token == "$") {
    return pos == input_line.size() && remaining_tokens.empty();
  } else if (current_token[0] == '(') {
    size_t colon_pos = current_token.find(':');
    if (colon_pos == std::string::npos || current_token.back() != ')') {
      throw std::runtime_error("Invalid group token: " + current_token);
    }
    std::string group_num_str = current_token.substr(1, colon_pos - 1);
    std::string group_content = current_token.substr(colon_pos + 1, current_token.size() - colon_pos - 2 - (current_token.back() == '+' || current_token.back() == '?' ? 1 : 0));
    size_t groupStart = pos;
    std::vector<std::string> alternatives = split(group_content);
    size_t savedPos = pos;
    for (auto &alt : alternatives) {
      pos = savedPos;
      int temp_group_counter = 0; // Prevent modifying the original group_counter
      std::vector<std::string> groupTokens = tokenize_pattern(alt, temp_group_counter);
      std::unordered_map<std::string, std::string> temp_groups;
      if (match_from(groupTokens, input_line, pos, temp_groups, temp_group_counter)) {
        groups[group_num_str] = input_line.substr(groupStart, pos - groupStart);
        if (match_from(remaining_tokens, input_line, pos, groups, group_counter)) {
          return true;
        }
      }
    }
    return false;
  } else if (current_token.size() >= 2 && current_token[0] == '\\' && isdigit(current_token[1])) {
    std::string group_num_str = current_token.substr(1);
    auto it = groups.find(group_num_str);
    if (it == groups.end()) return false;
    std::string expected = it->second;
    if (input_line.substr(pos, expected.length()) != expected) return false;
    pos += expected.length();
    return match_from(remaining_tokens, input_line, pos, groups, group_counter);
  } else if (current_token.back() == '+' || current_token.back() == '?') {
    return handle_quantifier(remaining_tokens, current_token, pos, input_line, groups, group_counter);
  } else {
    if (!match_token(pos, input_line, current_token)) return false;
    pos++;
    return match_from(remaining_tokens, input_line, pos, groups, group_counter);
  }
}

bool match_pattern(const std::string &input_line, const std::vector<std::string> &tokens, int &group_counter) {
  if (tokens.empty()) return input_line.empty();

  bool anchored_start = (!tokens.empty() && tokens[0] == "^");
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
