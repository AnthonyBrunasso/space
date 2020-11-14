#pragma once

#include <cassert>
#include <vector>

#include "common/common.cc"

namespace alng {

DECLARE_ARRAY(ASTNode, 256);

void ASTSwapNodes(ASTNode* l, ASTNode* r) {
  ASTNode t = *l;
  *l = *r;
  *r = t;
}

ASTNode* ASTCreateNode(const Token& token) {
  ASTNode* node = UseASTNode();
  switch (token.type) {
    case Token::kIntLiteral: {
      node->type = ASTNode::kIntLiteral;
      node->int_literal = token.int_literal;
    } break;
    case Token::kOperator: {
      node->type = ASTNode::kArithmetic;
      node->arithmetic_params.op = token.op;
      node->arithmetic_params.lhs = nullptr;
      node->arithmetic_params.rhs = nullptr;
    } break;
    case Token::kSeperator:
    default: assert(!"Implement ASTCreateNode");
  }
  return node;
}

bool ASTInsertNode(Lexer* lexer, ASTNode** root_node) {
  Token token;
  int cursor = lexer->cursor();
  if (!lexer->AdvanceCursor(&token)) return false;

  if (token.type == Token::kSeperator && token.seperator == '(') {
    ASTNode* subexpr_root = nullptr;
    while (ASTInsertNode(lexer, &subexpr_root));
    if (!(*root_node)) *root_node = subexpr_root;
    else {
      if ((*root_node)->type == ASTNode::kArithmetic) {
        if (!(*root_node)->arithmetic_params.lhs) {
          (*root_node)->arithmetic_params.lhs = subexpr_root;
        } else if (!(*root_node)->arithmetic_params.rhs) {
          (*root_node)->arithmetic_params.rhs = subexpr_root;
        } else {
          // TODO: This is probably not going to work in all cases.
          (*root_node)->arithmetic_params.rhs->arithmetic_params.rhs =
              subexpr_root;
        }
      }
    }
    return true;
  }

  if (token.type == Token::kSeperator && token.seperator == ')') {
    return false; 
  }

  ASTNode* node = ASTCreateNode(token);
  if (!(*root_node)) {
    *root_node = node;
    return true;
  }

  // Root is a literal like and new node is arithmetic: "2 + 3 + 4"
  if ((*root_node)->type == ASTNode::kIntLiteral &&
      node->type == ASTNode::kArithmetic) {
    node->arithmetic_params.lhs = *root_node;
    *root_node = node;
    return true;
  }

  // Simple case that the root is arithmetic and is missing its rhs param.
  if ((*root_node)->type == ASTNode::kArithmetic &&
      node->type == ASTNode::kIntLiteral) {
    if (!(*root_node)->arithmetic_params.rhs) {
      (*root_node)->arithmetic_params.rhs = node;
    } else {
      (*root_node)->arithmetic_params.rhs->arithmetic_params.rhs = node;
    }
    return true;
  }

  // Replace root - effectively add a lower precendence op / rotate tree.
  if ((*root_node)->type == ASTNode::kArithmetic &&
      ASTGetPrecedence(node) == 1) {
    node->arithmetic_params.lhs = *root_node;
    *root_node = node;
    return true;
  }

  // Traverse into rhs of tree, pushing higher precedence op down.
  if ((*root_node)->type == ASTNode::kArithmetic &&
      ASTGetPrecedence(node) == 2) {
    if ((*root_node)->arithmetic_params.rhs) {
      ASTNode* t = (*root_node)->arithmetic_params.rhs;
      (*root_node)->arithmetic_params.rhs = node;
      node->arithmetic_params.lhs = t;
    }
    return true;
  }

  return false;
}

bool ASTParse(Lexer* lexer, ASTNode** root_node) {
  lexer->Reset();
  while (lexer->has_input()) {
    if (!ASTInsertNode(lexer, root_node)) return false;
  }
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
