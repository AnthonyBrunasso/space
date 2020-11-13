#include "alang.cc"

#include <vector>

const char* kExample_1 = "13 + 3 - 5 * 2 - 5 * 3";
//                        16 - 10 - 15 = -9
const char* kExample_2 = "13 + 3 - 7";
const char* kExample_3 = "2 - 13 + 3 - 7 + 45 + 3 - 8";
const char* kExample_4 = "13 + 3 * 2 + 5";
const char* kExample_5 = "13 + 3 + 2 * 3 * 6 * 2 + 3 - 3 * 2 + 8 * 9";

int main(int argc, char** argv) {
  alng::Lexer lexer(kExample_5, strlen(kExample_5));
  alng::ASTNode* root = nullptr;
  alng::ASTParse(&lexer, &root);
  if (root) printf("%s\n", root->DebugString().c_str());
  if (root) printf("evaluates to %i\n", alng::ASTEvaluate(root));
  return 0;
}
