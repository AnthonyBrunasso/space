#include "alang.cc"

#include <cassert>

void Test_1() {
  const char* kExample = "13 + 3 - 5 * 2 - 5 * 3";
  alng::Lexer lexer(kExample, strlen(kExample));
  alng::ASTNode* root = nullptr;
  alng::ASTParse(&lexer, &root);
  assert(alng::ASTEvaluate(root) == -9);
}

void Test_2() {
  const char* kExample = "13 + 3 - 7";
  alng::Lexer lexer(kExample, strlen(kExample));
  alng::ASTNode* root = nullptr;
  alng::ASTParse(&lexer, &root);
  assert(alng::ASTEvaluate(root) == 9);
}

void Test_3() {
  const char* kExample = "2 - 13 + 3 - 7 + 45 + 3 - 8";
  alng::Lexer lexer(kExample, strlen(kExample));
  alng::ASTNode* root = nullptr;
  alng::ASTParse(&lexer, &root);
  assert(alng::ASTEvaluate(root) == 25);
}

void Test_4() {
  const char* kExample = "13 + 3 * 2 + 5";
  alng::Lexer lexer(kExample, strlen(kExample));
  alng::ASTNode* root = nullptr;
  alng::ASTParse(&lexer, &root);
  assert(alng::ASTEvaluate(root) == 24);
}

void Test_5() {
  const char* kExample = "13 + 3 + 2 * 3 * 6 * 2 + 3 - 3 * 2 + 8 * 9";
  alng::Lexer lexer(kExample, strlen(kExample));
  alng::ASTNode* root = nullptr;
  alng::ASTParse(&lexer, &root);
  assert(alng::ASTEvaluate(root) == 157);
}

void RunAllTests() {
  Test_1();
  Test_2();
  Test_3();
  Test_4();
  Test_5();
}

int main(int argc, char** argv) {
  RunAllTests();
  printf("Tests passed\n");
  return 0;
}
