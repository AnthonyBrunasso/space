#include <cassert>
#include <cstdio>
#include <vector>

/*


sub
  add
    5
    mul
      5
      2
  3


5 + 5 - 3

sub
  add
    5
    5
  3

5 * (5 - 2)

mul
  5
  sub
    5
    2

5 * 5 - 2

sub
  mul
    5
    5
  2

2 + ((7 - 3) * 2) + (4 - 2) * 6

add
  add
    2
    subexpression
      mul
        subexpression
          sub
            7
            3
        2
  mul
    subexpression
      sub
        4
        2

5 * (3 - 2) - (7 * 2) + 3

5 * (1) - (14) + 3
5 - 14 + 3
-9 + 3
-6

Expression

  Operation
  Literal

*/

struct Expression {
  enum Type {
    kLiteral,
    kAddition,
    kSubtraction,
    kMultiplication,
    kSubexpression,
  };
  Type op;
  Expression* lhs = nullptr;
  Expression* rhs = nullptr;
  Expression* subexpression = nullptr;
  int val = 0;
};

void PrintExpr(Expression* expr, int indent_lvl = 0) {
  if (expr == nullptr) return;
  switch (expr->op) {
    case Expression::kLiteral: {
      static const char* kLiteral = "Literal";
      printf("%*s(%i)\n", indent_lvl + strlen(kLiteral), kLiteral, expr->val);
    } break;
    case Expression::kAddition: {
      static const char* kAdd = "Add";
      printf("%*s\n", indent_lvl + strlen(kAdd), kAdd);
      PrintExpr(expr->lhs, indent_lvl + 1);
      PrintExpr(expr->rhs, indent_lvl + 1);
    } break;
    case Expression::kSubtraction: {
      static const char* kSub = "Sub";
      printf("%*s\n", indent_lvl + strlen(kSub), kSub);
      PrintExpr(expr->lhs, indent_lvl + 1);
      PrintExpr(expr->rhs, indent_lvl + 1);
    } break;
    case Expression::kMultiplication: {
      static const char* kMul = "Mul";
      printf("%*s\n", indent_lvl + strlen(kMul), kMul);
      PrintExpr(expr->lhs, indent_lvl + 1);
      PrintExpr(expr->rhs, indent_lvl + 1);
    } break;
    case Expression::kSubexpression: {
      static const char* kSubexpression = "Subexpression";
      printf("%*s\n", indent_lvl + strlen(kSubexpression), kSubexpression);
      PrintExpr(expr->subexpression, indent_lvl + 1);
    } break;
  }
}

int EvaluateExpr(Expression* expr) {
  if (!expr) return 0; // Error
  switch (expr->op) {
    case Expression::kLiteral: {
      return expr->val;
    } break;
    case Expression::kAddition: {
      return EvaluateExpr(expr->lhs) + EvaluateExpr(expr->rhs);
    } break;
    case Expression::kSubtraction: {
      return EvaluateExpr(expr->lhs) - EvaluateExpr(expr->rhs);
    } break;
    case Expression::kMultiplication: {
      return EvaluateExpr(expr->lhs) * EvaluateExpr(expr->rhs);
    } break;
    case Expression::kSubexpression: {
      return EvaluateExpr(expr->subexpression);
    } break;
  }
  return 0;
}

bool ConstructTreeFromInput(Expression*& expr, Expression*& op,
                            Expression*& lhs, Expression*& rhs) {
  int int_val;
  char char_val;
  if (scanf("%i", &int_val) == 1) {
    //printf("GOT INT VAL %i\n", int_val);
    if (lhs == nullptr) {
      lhs = new Expression;
      lhs->op = Expression::kLiteral;
      lhs->val = int_val;
     // printf("SETTING LHS %i\n", int_val);
    } else if (rhs == nullptr) {
      rhs = new Expression;
      rhs->op = Expression::kLiteral;
      rhs->val = int_val;
      if (!op) { // issue
        printf("Handle error...\n");
        return 1;
      }
      op->rhs = rhs;
    }
  } else if (scanf("%c", &char_val) == 1) {
    // For terminating subquery generation.
    if (char_val == ')') return false;
    op = new Expression;
    if (char_val == '+') {
      op->op = Expression::kAddition;
      op->lhs = lhs;
    } else if (char_val == '-') {
      op->op = Expression::kSubtraction;
      op->lhs = lhs;
    } else if (char_val == '*') {
      op->op = Expression::kMultiplication;
      op->lhs = rhs != nullptr ? rhs : lhs;
    } else if (char_val == '(') {
      op->op = Expression::kSubexpression;
      op->lhs = lhs;
    }
    if (!expr) expr = op;
    else {
      if (char_val == '+' || char_val == '-') {
        lhs = expr;
        expr = op;
        expr->lhs = lhs;
        rhs = nullptr;
      } else if (char_val == '*') {
        expr->rhs = op;
        rhs = nullptr;
      } else if (char_val == '(') {
        Expression* subexpr = nullptr;
        Expression* new_op = nullptr;
        Expression* new_lhs = nullptr;
        Expression* new_rhs = nullptr;
        while (ConstructTreeFromInput(subexpr, new_op, new_lhs, new_rhs));
        op->subexpression = subexpr;
        if (!expr->lhs) expr->lhs = op;
        if (!expr->rhs) expr->rhs = op;
      }
    }
  }
  //PrintExpr(expr);
  return true;
}

int main(int argc, char** argv) {
  Expression* root_expr = nullptr;
  Expression* op = nullptr;
  Expression* lhs = nullptr;
  Expression* rhs = nullptr;

  while (ConstructTreeFromInput(root_expr, op, lhs, rhs)) {
    PrintExpr(root_expr);
    printf("Result: %i\n", EvaluateExpr(root_expr));
  }

  return 0;
}
