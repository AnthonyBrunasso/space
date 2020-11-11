#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

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

void AdvanceTokenizer(const char* txt, Token* token, int* idx) {
  if (!token->identifier_ptr) {
    token->identifier_ptr = &txt[*idx];
  }
  ++token->identifier_size;
  ++(*idx);
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
    while (has_input() && text_[cursor_] == ' ') {
      ++cursor_;
    }
  }

  bool AdvanceCursor(Token* token) {
    if (!has_input()) return false;
    *token = {};
    while (IsLiteral(character())) {
      AdvanceTokenizer(text_, token, &cursor_);
      if (cursor_ >= text_size_ || !IsLiteral(character())) {
        char* end;
        token->type = Token::kIntLiteral;
        token->literal = strtol(token->identifier_ptr, &end, 10);
        SkipWhitespace();
        return true;
      }
    }

    while (IsOperator(character())) {
      AdvanceTokenizer(text_, token, &cursor_);
      if (cursor_ >= text_size_ || !IsOperator(character())) {
        token->type = Token::kOperator;
        token->op = *token->identifier_ptr;
        SkipWhitespace();
        return true;
      }
    }

    while (IsSeperator(character())) {
      AdvanceTokenizer(text_, token, &cursor_);
      if (cursor_ >= text_size_ || !IsSeperator(character())) {
        token->type = Token::kSeperator;
        token->seperator = *token->identifier_ptr;
        SkipWhitespace();
        return true;
      }
    }

    if (has_input()) printf("Unknown char %c\n", character());
    return false;
  }

 private:
  const char* text_;
  int text_size_;
  int cursor_;
};

}
