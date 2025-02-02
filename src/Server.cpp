#include <cctype>
#include <cerrno>
#include <cstddef>
#include <ios>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

// TODO: make parsing char groups more efficient, currently does it once per
// input_line character, but should only need to process it once.
std::vector<std::string> tokenize_pattern(std::string &pattern) {
  std::vector<std::string> tokens;

  std::cout << "input pattern: '" << pattern << "'\n";

  for (size_t i = 0; i < pattern.length(); i++) {
    std::cout << "\nProcessing index: " << i << ", char: '" << pattern[i]
              << "'\n";
    if (pattern[i] == '\\') {
      std::cout << "'\\' found at index: " << i << "\n";
      // handle char class
      if (i + 1 < pattern.length()) {
        std::string escaped_token = pattern.substr(i, 2);
        std::cout << "Adding escaped token: '" << escaped_token << "'\n";
        tokens.push_back(escaped_token); // handle \d, \w with substr of 2
        i++;
      } else {
        std::cout << "WARNING: escape character at end of pattern\n";
      }
    } else if (pattern[i] == '[') {
      std::cout << "'[' found at index: " << i << "\n";
      size_t end = pattern.find(']', i); // index of matching ]
      if (end == std::string::npos) {    // matching not found
        throw std::runtime_error("Matching ']' not found in: " + pattern);
      }
      std::string token =
          pattern.substr(i, end - i + 1); // inclusively tokenize char group
      std::cout << "Adding character class token: '" << token
                << "', end index: " << end << "\n";
      tokens.push_back(token);
      i = end; // move index up
    } else {

      std::string single_char(1, pattern[i]);
      if (i + 1 < pattern.size() && pattern[i + 1] == '+') {
        std::string one_more = pattern.substr(i, 2);
        std::cout << "Adding '" << one_more;
        tokens.push_back(one_more);
        i++;
      } else {
        std::cout << "Adding single character token: '" << single_char << "'\n";
        tokens.push_back(single_char); // add token size 1, char c = p[i]
      }
    }
  }
  std::cout << "\nFinal tokens:\n";

  for (size_t i = 0; i < tokens.size(); i++) {
    std::cout << i << ": '" << tokens[i] << "'\n";
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

bool match_token(size_t idx, const std::string &input_line,
                 const std::string &token) {
  char c = input_line[idx];

  if (token.size() == 2 && token[token.size() - 1] == '+') {
    char c_one = token[0];
    std::cout << "Matching one or more operator on '" << c_one << "'\n";
    size_t m = idx;
    if (c_one != c) {
      std::cout << "'" << c_one << "' doesn't match '" << c << "'\n";
      return false;
    }
    while (input_line[m] == c_one) {
      std::cout << m;
      m++;
    }
    return true;
  }
  if (token == "\\d") {
    return isdigit(c);
  }
  if (token == "$") { // place check before length() == 1
    if (c == '\0') {
      std::cout << "index is '\\0' at char: '" << input_line[idx] << "'\n";
      return true;
    }
    return false;
  }
  if (token == "\\w") {
    return isalpha(c);
  }
  if (token.length() == 1) {
    return token[0] == c;
  }
  if (token[0] == '[') {
    return match_char_group(c, token);
  }
  throw std::runtime_error("Unhandled token: " + token);
  return false;
}

bool match_from(size_t start_idx, const std::string &input_line,
                const std::vector<std::string> &tokens) {
  bool match = true;

  // iterate through tokens
  for (size_t i = 0; i < tokens.size(); i++) {

    // if (start_idx + i >= input_line.size()) {
    //   return false;
    // }

    char c = input_line[start_idx + i];

    std::string cur_token(tokens[i]);

    std::cout << "Matching '" << c << "' at index: " << (start_idx + i)
              << " on token: '" << cur_token << "'\n";

    if (cur_token == "^") {
      std::cout << "\nMatching start anchor with `match_here` function\n\n";
      const std::vector<std::string> inc_tokens(tokens.begin() + i,
                                                tokens.end());
      return match_from(start_idx + i, input_line, inc_tokens);
    }

    if (cur_token[cur_token.size() - 1] == '+') {
      char c_one = cur_token[0];
      std::cout << "Matching one or more operator on '" << c_one << "'\n";
      size_t m = i;
      if (c_one != c) {
        std::cout << "'" << c_one << "' doesn't match '" << c << "'\n";
        return false;
      }
      while (input_line[m] == c_one) {
        std::cout << m << " ";
        m++;
      }
      // matched until now, splice index to [:m], increment tokens, start index
      // = 0
      std::string new_string = input_line.substr(m, input_line.size());
      std::cout << "\nNew string after one or more matches: '" << new_string
                << "'\n";
      const std::vector<std::string> new_tokens(tokens.begin() + i + 1,
                                                tokens.end());
      for (size_t i = 0; i < new_tokens.size(); i++) {
        std::cout << i << " : " << new_tokens[i] << "\n";
      }

      return match_from(0, new_string, new_tokens);
    }
    if (!match_token(start_idx + i, input_line, cur_token)) {
      std::cout << "\nFailed to match on this iteration\n";
      return false;
    }
  }
  if (match) {
    return true;
  }
  std::cout << "\nReturning false. Failed to match in `match_here`\n";
  return false;
}

bool match_pattern(const std::string &input_line,
                   const std::vector<std::string> &tokens) {

  std::cout << "\nLength of '" << input_line << "': " << input_line.size()
            << "\n";

  // need the pre-check here or else the '^' gets treated as a single char,
  // breaking the input length check
  if (tokens[0] == "^") {
    std::cout << "\nMatching start anchor with `match_here` function\n\n";
    const std::vector<std::string> inc_tokens(tokens.begin() + 1, tokens.end());
    return match_from(0, input_line, inc_tokens);
  }
  if (tokens[tokens.size() - 1] == "$") {
    std::cout << "\nMatching end anchor\n\n";
    return match_from(0, input_line, tokens);
  }

  // outer loop for each possible starting pos in input
  for (size_t start = 0; start + tokens.size() <= input_line.length();
       start++) {
    bool match = true;
    std::cout << "\n-- Starting '" << input_line[start] << "': " << start
              << ", '" << input_line << "' ...\n\n";

    match = match_from(start, input_line, tokens);
    if (match) {
      return true;
    }
  }
  std::cout << "Returning false from `match_pattern`\n\n";
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
