#pragma once

#include <cassert>
#include <vector>

namespace alng {

bool ASTParse(const std::vector<Token>& tokens, ASTExpression* root_expr) {
  return true;
}

int ASTEvaluate(ASTExpression* expr) {
  if (!expr) return 0;
  switch (expr->type) {
    case ASTExpression::Type::kNull:
    case ASTExpression::Type::kIntLiteral: {
      return expr->value;
    } break;
    case ASTExpression::Type::kArithmeticAdd: {
      assert(expr->children.size() == 2);
      return ASTEvaluate(&expr->children[0]) + ASTEvaluate(&expr->children[1]);
    } break;
    case ASTExpression::Type::kArithmeticSubtract: {
      assert(expr->children.size() == 2);
      return ASTEvaluate(&expr->children[0]) - ASTEvaluate(&expr->children[1]);
    } break;
    case ASTExpression::Type::kArithmeticMultiply: {
      assert(expr->children.size() == 2);
      return ASTEvaluate(&expr->children[0]) * ASTEvaluate(&expr->children[1]);
    } break;
    case ASTExpression::Type::kArithmeticDivide: {
      assert(expr->children.size() == 2);
      return ASTEvaluate(&expr->children[0]) / ASTEvaluate(&expr->children[1]);
    } break;
    case ASTExpression::Type::kSubexpression: {
      assert(expr->children.size() == 1);
      return ASTEvaluate(&expr->children[0]);
    } break;
    default: assert(!"Missing ASTExpression::Type from ASTEvaluate");
  }
}

}
