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
  const char* kExample = "1 * (3 + 2 / 1 * (1 + 3)) * 3 - 1 + (6 / 2 + 1 * (2 + 1 * (3 + 1)))";
  alng::Lexer lexer(kExample, strlen(kExample));
  alng::ASTNode* root = nullptr;
  alng::ASTParse(&lexer, &root);
  assert(alng::ASTEvaluate(root) == 41);
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

void Test_6() {
  const char* kExample = "4 * (6 / (1 + 2)) - 3 + 5 * (2 - 4)";
  alng::Lexer lexer(kExample, strlen(kExample));
  alng::ASTNode* root = nullptr;
  alng::ASTParse(&lexer, &root);
  assert(alng::ASTEvaluate(root) == -5);
}

void Test_7() {
  const char* kExample = "(3 - 2)";
  alng::Lexer lexer(kExample, strlen(kExample));
  alng::ASTNode* root = nullptr;
  alng::ASTParse(&lexer, &root);
  assert(alng::ASTEvaluate(root) == 1);
}

void RunAllTests() {
  Test_1();
  Test_2();
  Test_3();
  Test_4();
  Test_5();
  Test_6();
  Test_7();
}

int main(int argc, char** argv) {
  RunAllTests();
  printf("Tests passed\n");
  return 0;
}
