#pragma once 

#include <cassert>
#include <string>
#include <sstream>
#include <variant>
#include <vector>

namespace alng {

struct Token {
  enum Type {
    kIntLiteral,    // 5 23
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


struct ASTExpression {
  enum Type {
    kNull,
    kIntLiteral,
    kArithmeticAdd,
    kArithmeticSubtract,
    kArithmeticMultiply,
    kArithmeticDivide,
    kSubexpression,
  };

  Type type = kNull;
  
  std::variant<int, std::vector<ASTExpression>> data;

  int& int_literal() {
    return std::get<0>(data);
  }

  std::vector<ASTExpression>& children() {
    return std::get<1>(data);
  }

  int int_literal() const {
    return std::get<0>(data);
  }

  const std::vector<ASTExpression>& children() const {
    return std::get<1>(data);
  }

  std::string DebugString(int lvl = 0) const {
    std::ostringstream str;
    str.write(" ", lvl * 2);
    switch (type) {
      case kNull: {
        str << "Node(NULL)" << std::endl;
      } break;
      case kIntLiteral: {
        str << "Node(Literal int_value: " << int_literal() << ")"
            << std::endl;
      } break;
      case kArithmeticAdd: {
        str << "Node(Arithmetic ADD)" << std::endl;
        for (const auto& child : children()) {
          str << child.DebugString(lvl + 1);
        }
      } break;
      case kArithmeticSubtract: {
        str << "Node(Arithmetic SUBTRACT)" << std::endl;
        for (const auto& child : children()) {
          str << child.DebugString(lvl + 1);
        }
      } break;
      case kArithmeticMultiply: {
        str << "Node(Arithmetic MULTIPLY)" << std::endl;
        for (const auto& child : children()) {
          str << child.DebugString(lvl + 1);
        }
      } break;
      case kArithmeticDivide: {
        str << "Node(Arithmetic DIVIDE)" << std::endl;
        for (const auto& child : children()) {
          str << child.DebugString(lvl + 1);
        }
      } break;
      case kSubexpression: {
        str << "Node(Subexpression)" << std::endl;
        for (const auto& child : children()) {
          str << child.DebugString(lvl + 1);
        }
      } break;
      default: assert(!"Missing type in ASTExpression::DebugString");
    }
    return str.str();
  }
};

}
