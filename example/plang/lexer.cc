#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sstream>

struct Token {
  enum Type {
    kLiteral,    // 5 23
    kOperator,   // + - * /
    kSeperator,  // ( ) , ; 
  };
  // Points to the starting location of the identifier.
  const char* identifier_ptr;
  // Length of the identifier.
  int identifier_size;
  // Type of token see Token::Type.
  Type type;
  union {
    int literal;
    char op;
    char seperator;
  };

  std::string DebugString() const {
    std::ostringstream str;
    str << "[identifier(\"";
    str.write(identifier_ptr, identifier_size);
    str << "\"), ";
    switch (type) {
      case kLiteral: {
        str << "literal(" << literal << ")]";
      } break;
      case kOperator: {
        str << "operator(" << op << ")]";
      } break;
      case kSeperator: {
        str << "seperator('" << seperator << "')]";
      } break;
      default: assert(!"Missing type in Token::DebugString");
    }
    return str.str();
  }
};

const char* kExample_1 = "13 + 3 - (5 * 2)";

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
      token->type = Token::kLiteral;
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

int main(int argc, char** argv) {
  int len = strlen(kExample_1);
  int idx = 0;
  SkipWhitespace(kExample_1, len, &idx);

  Token token;
  while (ParseToken(kExample_1, len, &token, &idx)) {
    printf("%s\n", token.DebugString().c_str());
    SkipWhitespace(kExample_1, len, &idx);
  }
  
  return 0;
}
