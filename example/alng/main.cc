#include "alang.cc"

#include <vector>

const char* kExample_1 = "13 + 3 - (5 * 2)";
const char* kExample_2 = "13 + 3 - 7";

int main(int argc, char** argv) {
  std::vector<alng::Token> tokens = alng::Tokenize(kExample_2);
  for (const auto& t : tokens) {
    printf("%s\n", t.DebugString().c_str());
  }  
  alng::ASTExpression root_expr;
  alng::ASTParse(tokens, &root_expr);
  printf("%s\n", root_expr.DebugString().c_str());
  return 0;
}
