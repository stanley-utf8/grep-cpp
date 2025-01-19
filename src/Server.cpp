#include <cctype>
#include <iostream>
#include <regex>
#include <string>
#include <unordered_map>
#include <unordered_set>

bool match_pattern(const std::string &input_line, const std::string &pattern) {
  std::cerr << "inside match_pattern" << std::endl;
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
    std::cerr << "procesing \\w" << std::endl;
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
    std::cerr << "Processing character group\n";
    if (pattern[1] == '^') {
      int idx = 2;
      std::unordered_set<char> char_set;
      std::cerr << "Processing negative character group\n";
      while (idx < pattern.length() - 1) {
        char cur = pattern[idx];
        char next = pattern[idx + 1];
        std::cerr << "Current char: " << cur << ", Next char: " << next << "\n";
        std::cerr << "Current idx: " << idx << "\n";

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

      for (const auto &l : input_line) {
        if (char_set.find(l) != char_set.end()) {
          return false;
        }
      }
      return true;

    } else {
      int idx = 1;
      std::unordered_set<char> char_set;
      std::cerr << "Processing positive character group\n";
      while (idx < pattern.length() - 1) {
        char cur = pattern[idx];
        char next = pattern[idx + 1];
        std::cerr << "Current char: " << cur << ", Next char: " << next << "\n";
        std::cerr << "Current idx: " << idx << "\n";

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

      for (const auto &l : input_line) {
        if (char_set.find(l) != char_set.end()) {
          return true;
        }
      }
      return false;

  }
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
