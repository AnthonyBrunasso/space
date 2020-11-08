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

void SkipWhitespace(const char* txt, int txt_size, int* idx) {
  if (*idx >= txt_size) return;
  while (*idx < txt_size && txt[(*idx)] == ' ') {
    (*idx)++;
  }
}

void AdvanceTokenizer(const char* txt, Token* token, int* idx) {
  if (!token->identifier_ptr) {
    token->identifier_ptr = &txt[*idx];
  }
  ++token->identifier_size;
  ++(*idx);
}

// Fills token with the first token starting at *idx and modifies idx
// to point to the the space following the identifier.
bool ParseToken(const char* txt, int txt_size, Token* token, int* idx) {
  *token = {};
  if (*idx >= txt_size) return false;
  while (IsLiteral(txt[*idx])) {
    AdvanceTokenizer(txt, token, idx);
    if ((*idx) >= txt_size || !IsLiteral(txt[*idx])) {
      token->type = Token::kIntLiteral;
      char* end;
      token->literal = strtol(token->identifier_ptr, &end, 10);
      return true;
    }
  }

  while (IsOperator(txt[*idx])) {
    AdvanceTokenizer(txt, token, idx);
    if ((*idx) >= txt_size || !IsOperator(txt[*idx])) {
      token->type = Token::kOperator;
      token->op = *token->identifier_ptr;
      return true;
    }
  }

  while (IsSeperator(txt[*idx])) {
    AdvanceTokenizer(txt, token, idx);
    if ((*idx) >= txt_size || !IsSeperator(txt[*idx])) {
      token->type = Token::kSeperator;
      token->seperator = *token->identifier_ptr;
      return true;
    }
  }

  if (*idx < txt_size) printf("Unknown char %c\n", txt[*idx]);
  return false;
}

std::vector<Token> Tokenize(const char* txt) {
  std::vector<Token> tokens;
  int len = strlen(txt);
  int idx = 0;
  SkipWhitespace(txt, len, &idx);
  Token token;
  while (ParseToken(txt, len, &token, &idx)) {
    SkipWhitespace(txt, len, &idx);
    tokens.push_back(token);
  }
  return tokens;
}

}
