#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

namespace alng {

inline bool IsLiteral(char c) {
  return c >= '0' && c <= '9';
}

inline bool IsOperator(char c) {
  return c == '+' || c == '-' || c == '*' || c == '/' || c == '=';
}

inline bool IsSeperator(char c) {
  return c == '(' || c == ')' || c == ',' || c == ';';
}

inline bool IsWhitespace(char c) {
  return c == ' ' || c == '\n' || c == '\t';
}

inline bool IsValidSymbolCharacter(char c) {
  if (c > 'a' && c < 'z') return true;
  if (c > 'A' && c < 'Z') return true;
  return false;
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

  bool IfTypeSetToken(Token* token) {
    if (cursor_ >= text_size_) return false;
    if (cursor_ + 4 <= text_size_ &&
        strncmp(&text_[cursor_], "int ", 4) == 0) {
      token->identifier_ptr = &text_[cursor_];
      token->identifier_size = 3;
      token->alg_type = AlgType::kInt;
      cursor_ += 4;
      return true;
    }
    if (cursor_ + 5 <= text_size_ &&
        strncmp(&text_[cursor_], "uint ", 5) == 0) {
      token->identifier_ptr = &text_[cursor_];
      token->identifier_size = 4;
      token->alg_type = AlgType::kUint;
      cursor_ += 5;
      return true;
    }
    if (cursor_ + 5 <= text_size_ &&
        strncmp(&text_[cursor_], "char ", 5) == 0) {
      token->identifier_ptr = &text_[cursor_];
      token->identifier_size = 4;
      token->alg_type = AlgType::kChar;
      cursor_ += 5;
      return true;
    }
    return false;
  }

  bool IfSymbolSetToken(Token* token) {
    char c = character();
    if (!IsValidSymbolCharacter(c)) return false;
    if (IsSeperator(c) || IsOperator(c) || IsLiteral(c)) return false;
    token->identifier_ptr = &text_[cursor_];
    while (cursor_ <= text_size_ && IsValidSymbolCharacter(character())) {
      ++token->identifier_size;
      ++cursor_;
    }
    return true;
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
        token->int_literal = strtol(token->identifier_ptr, &end, 10);
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

    if (IsSeperator(character())) {
      SetAndAdvance(token);
      token->type = Token::kSeperator;
      token->seperator = *token->identifier_ptr;
      return true;
    }

    if (IfTypeSetToken(token)) {
      token->type = Token::kType;
      return true;
    }

    if (IfSymbolSetToken(token)) {
      token->type = Token::kSymbol;
      return true;
    }

    if (has_input()) printf("Unknown char %c\n", character());
    return false;
  }

  void Reset() {
    cursor_ = 0;
  }

  int cursor() const {
    return cursor_;
  }

  void set_cursor(int cursor) {
    cursor_ = cursor;
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
