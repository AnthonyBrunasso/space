#pragma once

#include <cassert>
#include <vector>

#include "common/common.cc"

namespace alng {

DECLARE_ARRAY(ASTNode, 128);

void ASTSwapNodes(ASTNode* l, ASTNode* r) {
  ASTNode t = *l;
  *l = *r;
  *r = t;
}

bool ASTParse(Lexer* lexer, ASTNode** root_node, ASTNode* curr_node, int priority) {
  Token token;
  int cursor = lexer->cursor();
  while (lexer->AdvanceCursor(&token)) {
    switch (token.type) {
      case Token::kIntLiteral: {
        if (!(*root_node)) {
          *root_node = UseASTNode();
          (*root_node)->type = ASTNode::kIntLiteral;
          (*root_node)->int_literal = token.literal;
          curr_node = *root_node;
        } else {
          // TODO: Check that curr is an appropriate node for this operation.
          if (curr_node->type == ASTNode::kArithmetic) {
            assert(curr_node->arithmetic_params.lhs);
            ASTNode* rhs = UseASTNode();
            rhs->type = ASTNode::kIntLiteral;
            rhs->int_literal = token.literal;
            curr_node->arithmetic_params.rhs = rhs;
          }
        }
      } break;
      case Token::kOperator: {
        if (!root_node) assert(!"Bad expression can't begin with operator");
        // TODO: Check that curr is an appropriate node for this operation.
        int precedence = GetOperatorPrecedence(token.op);
        if (precedence > priority) {
          lexer->set_cursor(cursor);
          if (!ASTParse(lexer, &curr_node->arithmetic_params.rhs,
                        curr_node->arithmetic_params.rhs, precedence)) {
            return false;
          }
          return true;
        } else {
          ASTNode* new_node = UseASTNode();
          new_node->type = ASTNode::kArithmetic;
          new_node->arithmetic_params.op = token.op;
          ASTSwapNodes(new_node, curr_node);
          // curr_node is now the arithmetic expression.
          curr_node->arithmetic_params.lhs = new_node;
          curr_node->arithmetic_params.rhs = nullptr;
        } 
      } break;
      case Token::kSeperator: {
      } break;
    }
    cursor = lexer->cursor();
  }
  return true;
}

bool ASTParse(Lexer* lexer, ASTNode** root_node) {
  lexer->Reset();
  if (!ASTParse(lexer, root_node, nullptr, 1)) return false;
  return true;
}

int ASTEvaluate(ASTNode* expr) {
  if (!expr) return 0;
  switch (expr->type) {
    case ASTNode::Type::kNull:
    case ASTNode::Type::kIntLiteral: {
      return expr->int_literal;
    } break;
    case ASTNode::Type::kArithmetic: {
      switch (expr->arithmetic_params.op) {
        case '+': {
          return ASTEvaluate(expr->arithmetic_params.lhs) +
                 ASTEvaluate(expr->arithmetic_params.rhs);
        } break;
        case '-': {
          return ASTEvaluate(expr->arithmetic_params.lhs) -
                 ASTEvaluate(expr->arithmetic_params.rhs);
        } break;
        case '*': {
          return ASTEvaluate(expr->arithmetic_params.lhs) *
                 ASTEvaluate(expr->arithmetic_params.rhs);
        } break;
        case '/': {
          return ASTEvaluate(expr->arithmetic_params.lhs) /
                 ASTEvaluate(expr->arithmetic_params.rhs);
        } break;
      }
    } break;
    case ASTNode::Type::kSubexpression: {
      return ASTEvaluate(expr->subexpression);
    } break;
    default: assert(!"Missing ASTExpression::Type from ASTEvaluate");
  }
  return 0;
}

}
