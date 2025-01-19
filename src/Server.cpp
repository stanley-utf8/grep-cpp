#include <cctype>
#include <iostream>
#include <regex>
#include <string>
#include <unordered_map>
#include <unordered_set>

bool match_pattern(const std::string &input_line, const std::string &pattern) {
  if (pattern.length() == 1) {
    return input_line.find(pattern) != std::string::npos;
  }

  if (pattern == "\\d") {
    // regex version
    // std::regex digit_pattern("[0-9]");
    // return std::regex_search(input_line, digit_pattern);
    for (char c : input_line) {
      if (isdigit(c)) {
        return true;
      }
    }
    return false;
  }

  if (pattern == "\\w") {
    // regex version
    // std::regex word_pattern("[a-z|A-Z|0-9|_]");
    // return std::regex_search(input_line, word_pattern);
    for (char c : input_line) {
      if (isalnum(c) || c == '_') {
        return true;
      }
    }
    return false;
  }

  if (pattern[0] == '[') {
    int idx = 1;
    std::unordered_set<char> char_set;
    while (idx + 1 < pattern.length() - 1 || pattern[idx + 1] == ']') {
      char cur = pattern[idx];
      char next = pattern[idx + 1];

      if (next == '-') {
        if (idx + 2 < pattern.length() - 1) {
          char_set.insert(cur);
          char_set.insert(next);
          idx += 2;
        } else {
          int start = cur - '0';
          int end = pattern[idx + 2] - '0';
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

    for (char c : input_line) {
      if (char_set.find(c) != char_set.end()) {
        return true;
      }
    }
    return false;

  }

  else {
    throw std::runtime_error("Unhandled pattern " + pattern);
  }
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
    if (match_pattern(input_line, pattern)) {
      return 0;
    } else {
      return 1;
    }
  } catch (const std::runtime_error &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
}
