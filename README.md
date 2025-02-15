[![progress-banner](https://backend.codecrafters.io/progress/grep/34dcb7a4-bf97-4da0-8e21-3686360057f2)](https://app.codecrafters.io/users/codecrafters-bot?r=2qF)

This is my attempt of the ["Build Your Own grep" Challenge](https://app.codecrafters.io/courses/grep/overview).

# Summary 

In this project, I built a simplified version of grep by designing a recursive backtracking parser for regular expressions. Instead of constructing an explicit finite-state machine (FSM) with predefined states and transitions, I implemented an implicit FSM using recursive pattern matching and position tracking. This approach allowed me to handle more complex regex features (such as nested groups and backreferences) naturally, however, backtracking can have performance trade-offs on highly ambiguous patterns.

For a simple example: 

In a traditional state-machine, `ab+c` might be matched as:

```
[State 0] --- a ---> [State 1] --- b ---> [State 2] --- b ---> [State 2] --- c ---> [State 3*]
```

* S0 is the initial state
* S1 is reached after seeing 'a'
* S2 is reached after seeing 'b' (and can loop on seeing more 'b's)
* S3 is the accepting state (marked with *) reached after seeing 'c'

In my implementation, `pos` acts like the current state, and the function `match_token()` checks for valid transitions, while 'b's are continually consumed within `handle_quantifier()`.

```cpp
if (quantifier == '+') {
  size_t count = 0;
  while (pos < input_line.size() &&
          match_token(pos, input_line, base_token)) {
    pos++;
    count++;
  }
  return count >= 1;
}
```

# Key Design Details

**Tokenization of Patterns**

I broke down regex patterns into tokens by scanning the input pattern string and recognizing special characters, character classes (`[abc]`, `[^def]`), escaped sequences (e.g., `\\d`, `\\w`), capturing groups (e.g., `(group)`), and quantifiers (`+`, `?`). This tokenization was crucial for later matching logic.

- **Data Structures Used**: I used vectors to store tokens and an incrementing counter to track capturing group numbers.
- **Error Handling**: I implemented runtime error checks for unmatched brackets or parentheses, ensuring robust parsing.
 
**Implicit FSM via Recursive Backtracking:**

Rather than building a state-machine with explicit states and transitions, I maintained a `pos` variable that acted as the current state. The matching function `match_token()` checked transitions by comparing tokens against the input string, while quantifiers were handled with `handle_quantifier()` which consumed one or more characters based on the quantifier rules.

- **Anchors**: Special tokens like `^` and `$` were handled to enforce matches at the beginning or end of the input.
- **Quantifiers**: The `+` quantifier is implemented greedily, ensuring at least one match, and the `?` quantifier optionally matches a token.

**Handling Character Classes and Groups:**

- **Character Classes**: I used `std::unordered_set` to manage character groups efficiently. The helper function `match_char_group()` dynamically builds a set of allowed (or disallowed) characters from patterns like `[a-z]` or `[^abc]`.
- **Capturing Groups**: Capturing groups are tokenized with a group number and stored in a `std::unordered_map` once matched. This enabled backreferences (e.g., \1), where the previously captured content is re-matched later in the input.


**Challenges**

I was unable to add support for nested backreferences, which was the last part of the codecrafters module on grep. 

I believe this was in-part due to a gun-ho attitude of starting to code before I knew the full-scope of the project. There were times during development I found myself needing to reorient functions that hadn't been fully thought out yet, which ultimately led to a messier implementation, and consequently made it much harder to add more complicated features down the line.
