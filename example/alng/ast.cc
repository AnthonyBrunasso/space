#pragma once

#include <cassert>
#include <vector>

namespace alng {

// 13 + 3 - 7
//
// 1.
// 13
//
// 2.
// add
//   13
//   3

struct ASTBuilder {
  ASTExpression* root_expr = nullptr;
  ASTExpression* current_expr = nullptr;
};

bool ASTAddToken(const Token& token, ASTBuilder* builder) {
  ASTExpression* root = builder->root_expr;
  ASTExpression* curr = builder->current_expr;
  switch (token.type) {
    case Token::Type::kIntLiteral: {
      if (root->type == ASTExpression::Type::kNull) {
        root->type = ASTExpression::Type::kIntLiteral;
        printf("setting literal... %i\n", token.literal);
        root->int_literal() = token.literal;
      } else {
      }
    } break;
    case Token::Type::kOperator: {
      if (curr->type == ASTExpression::Type::kNull) {
        printf("Node type is kNull.\n");
        return false;
      }
      if (curr->type == ASTExpression::Type::kIntLiteral) {
#if 0
        printf("Making arithmetic add...\n");
        ASTExpression expr;
        switch (token.op) {
          case '+': {
            expr.type = ASTExpression::Type::kArithmeticAdd;
          } break;
        }
        printf("1\n");
        expr.children().push_back(*curr);
        printf("2\n");
        *curr = expr;
        printf("done...\n");
#endif
      }
    } break;
    case Token::Type::kSeperator: {
    } break;
    default: assert(!"Missing Token::Type from ASTAddToken");
  }
  return true;
}

bool ASTParse(const std::vector<Token>& tokens, ASTExpression* root_expr) {
  ASTBuilder builder;
  builder.root_expr = root_expr;
  builder.current_expr = root_expr;
  for (const Token& token : tokens) {
    if (!ASTAddToken(token, &builder)) {
      printf("Failed to construct AST...\n");
      return false;
    }
  }
  return true;
}

int ASTEvaluate(ASTExpression* expr) {
  if (!expr) return 0;
  switch (expr->type) {
    case ASTExpression::Type::kNull:
    case ASTExpression::Type::kIntLiteral: {
      return expr->int_literal();
    } break;
    case ASTExpression::Type::kArithmeticAdd: {
      assert(expr->children().size() == 2);
      return ASTEvaluate(&expr->children()[0]) + ASTEvaluate(&expr->children()[1]);
    } break;
    case ASTExpression::Type::kArithmeticSubtract: {
      assert(expr->children().size() == 2);
      return ASTEvaluate(&expr->children()[0]) - ASTEvaluate(&expr->children()[1]);
    } break;
    case ASTExpression::Type::kArithmeticMultiply: {
      assert(expr->children().size() == 2);
      return ASTEvaluate(&expr->children()[0]) * ASTEvaluate(&expr->children()[1]);
    } break;
    case ASTExpression::Type::kArithmeticDivide: {
      assert(expr->children().size() == 2);
      return ASTEvaluate(&expr->children()[0]) / ASTEvaluate(&expr->children()[1]);
    } break;
    case ASTExpression::Type::kSubexpression: {
      assert(expr->children().size() == 1);
      return ASTEvaluate(&expr->children()[0]);
    } break;
    default: assert(!"Missing ASTExpression::Type from ASTEvaluate");
  }
}

}
