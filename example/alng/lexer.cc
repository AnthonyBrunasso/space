#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

namespace alng {

inline bool IsLiteral(char c) {
  return c > '0' && c < '9';
}

inline bool IsOperator(char c) {
  return c == '+' || c == '-' || c == '*' || c == '/';
}

inline bool IsSeperator(char c) {
  return c == '(' || c == ')' || c == ',' || c == ';';
}

inline bool IsWhitespace(char c) {
  return c == ' ' || c == '\n' || c == '\t';
}

class Lexer {
 public:
  Lexer() = delete;
  Lexer(const char* text, int text_size) :
    text_(text), text_size_(text_size), cursor_(0) {}

  inline bool has_input() const {
    return cursor_ < text_size_; 
  }

  inline char character() const {
    assert(has_input());
    return text_[cursor_];
  }

  void SkipWhitespace() {
    if (!has_input()) return;
    while (has_input() && IsWhitespace(text_[cursor_])) {
      ++cursor_;
    }
  }

  bool AdvanceCursor(Token* token) {
    SkipWhitespace();
    if (!has_input()) return false;
    *token = {};
    while (IsLiteral(character())) {
      SetAndAdvance(token);
      if (cursor_ >= text_size_ || !IsLiteral(character())) {
        char* end;
        token->type = Token::kIntLiteral;
        token->literal = strtol(token->identifier_ptr, &end, 10);
        return true;
      }
    }

    while (IsOperator(character())) {
      SetAndAdvance(token);
      if (cursor_ >= text_size_ || !IsOperator(character())) {
        token->type = Token::kOperator;
        token->op = *token->identifier_ptr;
        return true;
      }
    }

    while (IsSeperator(character())) {
      SetAndAdvance(token);
      if (cursor_ >= text_size_ || !IsSeperator(character())) {
        token->type = Token::kSeperator;
        token->seperator = *token->identifier_ptr;
        return true;
      }
    }

    if (has_input()) printf("Unknown char %c\n", character());
    return false;
  }

 private:
  void SetAndAdvance(Token* token) {
    if (!token->identifier_ptr) {
      token->identifier_ptr = &text_[cursor_];
    }
    ++token->identifier_size;
    ++cursor_;
  }

  const char* text_;
  int text_size_;
  int cursor_;
};

}
