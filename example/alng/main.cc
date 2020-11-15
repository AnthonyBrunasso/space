#include "alang.cc"

#include <vector>

const char* kExample = R"(
  int x = 3;
)";

// add
//   3
//   5

// add
//   3
//   mul
//     5

int main(int argc, char** argv) {
  alng::Lexer lexer(kExample, strlen(kExample));
  alng::Token token;
  while (lexer.AdvanceCursor(&token)) {
    printf("token: %s\n", token.DebugString().c_str());
  }
  //alng::ASTNode* root = nullptr;
  //alng::ASTParse(&lexer, &root);
  //if (root) printf("%s\n", root->DebugString().c_str());
  //if (root) printf("evaluates to %i\n", alng::ASTEvaluate(root));
  return 0;
}
