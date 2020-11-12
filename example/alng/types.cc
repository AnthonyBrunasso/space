#pragma once 

#include <cassert>
#include <string>
#include <sstream>
#include <variant>
#include <vector>

namespace alng {

int GetOperatorPrecedence(char op) {
  switch (op) {
    case '+': return 1;
    case '-': return 1;
    case '*': return 2;
    case '/': return 2;
    default: assert(!"No operator precedence set.");
  };
  return 0;
}

struct Token {
  enum Type {
    kIntLiteral,  // 5 23
    kOperator,    // + - * /
    kSeperator,   // ( ) , ; 
  };
  // Points to the starting location of the identifier.
  const char* identifier_ptr;
  // Length of the identifier.
  int identifier_size;
  // Type of token see Token::Type.
  Type type;
  union {
    // TODO: Change to int_literal
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
      case kIntLiteral: {
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

struct ASTNode;

struct ASTArithmeticParams {
  char op;
  ASTNode* lhs;
  ASTNode* rhs;
};

struct ASTNode {
  enum Type {
    kNull,
    kIntLiteral,
    kArithmetic,
    kSubexpression,
  };

  Type type = kNull;

  union {
    int int_literal;
    ASTArithmeticParams arithmetic_params;
    ASTNode* subexpression;
  };

  std::string DebugString(int lvl = 0) const {
    std::ostringstream str;
    // TODO: This probably needs to change.
    for (int i = 0; i < lvl * 2; ++i) str << " ";
    switch (type) {
      case kNull: {
        str << "Node(NULL)" << std::endl;
      } break;
      case kIntLiteral: {
        str << "Node(Literal int_literal: " << int_literal << ")"
            << std::endl;
      } break;
      case kArithmetic: {
        str << "Node(Arithmetic operator: "
            << arithmetic_params.op << ")" << std::endl;
        if (arithmetic_params.lhs) {
          str << arithmetic_params.lhs->DebugString(lvl + 1);
        }
        if (arithmetic_params.rhs) {
          str << arithmetic_params.rhs->DebugString(lvl + 1);
        }
      } break;
      case kSubexpression: {
        str << "Node(Subexpression)" << std::endl;
        str << subexpression->DebugString(lvl + 1);
      } break;
      default: assert(!"Missing type in ASTNode::DebugString");
    }
    return str.str();
  }
};

}