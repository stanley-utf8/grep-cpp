#include <cctype>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

// TODO: make parsing char groups more efficient, currently does it once per
// input_line character, but should only need to process it once.
std::vector<std::string> tokenize_pattern(std::string &pattern) {
  std::vector<std::string> tokens;
  for (size_t i = 0; i < pattern.length(); i++) {
    if (pattern[i] == '\\') {
      // handle char class
      if (i + 1 < pattern.length()) {
        tokens.push_back(
            pattern.substr(i, 2)); // handle \d, \w with substr of 2
        i++;
      }
    } else if (pattern[i] == '[') {
      size_t end = pattern.find(']', i); // index of matching ]
      if (end == std::string::npos) {    // matching not found
        throw std::runtime_error("Matching ']' not found in: " + pattern);
      }
      std::string token =
          pattern.substr(i, end - i + 1); // inclusively tokenize char group
      tokens.push_back(token);
      i = end; // move index up
    } else {
      tokens.push_back(
          std::string(1, pattern[i])); // add token size 1, char c = p[i]
    }
  }
  return tokens;
}

bool match_char_group(char c, const std::string &pattern) {
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
        int end = pattern[idx + 2];
        std::cerr << "Adding range from " << start << " to " << end << "\n";
        for (char c = start; c <= end; c++) {
          char_set.insert(c);
        }
        idx += 3;
      }
    } else {
      char_set.insert(cur);
      idx += 1;
    }
  }
  std::cerr << "current char set:";
  for (char c : char_set) {
    std::cerr << c << " ";
  }
  std::cerr << "\n";

  if (is_negative) {
    return char_set.find(c) == char_set.end();
  } else {
    return char_set.find(c) != char_set.end();
  }
}

bool match_token(char c, const std::string &token) {
  if (token == "\\d") {
    return isdigit(c);
  } else if (token == "\\w") {
    return isalpha(c);
  } else if (token.length() == 1) {
    return token[0] == c;
  } else if (token[0] == '[') {
    return match_char_group(c, token);
  } else {
    throw std::runtime_error("Unhandled token: " + token);
  }
}

bool match_pattern(const std::string &input_line,
                   const std::vector<std::string> &tokens) {

  // outer loop for each possible starting pos in input
  for (size_t start = 0; start + tokens.size() <= input_line.length();
       start++) {
    bool match = true;

    // match patterns at cur position with inner loop to compare each token
    for (size_t i = 0; i < tokens.size(); i++) {
      if (!match_token(input_line[start + i], tokens[i])) {
        match = false;
        break;
      }
    }
    if (match) {
      return true;
    }
  }
  return false;
}

int main(int argc, char *argv[]) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  // You can use print statements as follows for debugging, they'll be visible
  // when running tests.
  std::cerr << "Logs from your program will appear here" << std::endl;

  if (argc != 3) {
    std::cerr << "Expected two arguments"
              << std::endl; // the flag and the pattern
    return 1;
  }

  std::string flag = argv[1];
  std::string pattern = argv[2];

  if (flag != "-E") {
    std::cerr << "Expected first argument to be '-E'" << std::endl;
    return 1;
  }

  std::string input_line;
  std::getline(std::cin, input_line); // << echo "..." -n

  try {
    std::vector<std::string> tokens = tokenize_pattern(pattern);
    if (match_pattern(input_line, tokens)) {
      return 0;
    } else {
      return 1;
    }
  } catch (const std::runtime_error &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
}
