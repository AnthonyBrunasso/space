#include "alang.cc"

#include <vector>

const char* kExample = "1 * (3 + 2 / 1 * (1 + 3)) * 3 - 1 + (6 / 2 + 1 * (2 + 1 * (3 + 1)))";

// add
//   3
//   5

// add
//   3
//   mul
//     5

int main(int argc, char** argv) {
  alng::Lexer lexer(kExample, strlen(kExample));
  alng::ASTNode* root = nullptr;
  alng::ASTParse(&lexer, &root);
  if (root) printf("%s\n", root->DebugString().c_str());
  if (root) printf("evaluates to %i\n", alng::ASTEvaluate(root));
  return 0;
}
