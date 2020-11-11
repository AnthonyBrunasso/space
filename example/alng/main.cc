#include "alang.cc"

#include <vector>

const char* kExample_1 = "13 + 3 - 5 * 2 - 5 * 3";
//                        16 - 10 - 15 = -9
const char* kExample_2 = "13 + 3 - 7";
const char* kExample_3 = "2 - 13 + 3 - 7 + 45 + 3 - 8";

int main(int argc, char** argv) {
  alng::Lexer lexer(kExample_1, strlen(kExample_1));
  while (lexer.has_input()) {
    alng::Token token;
    if (!lexer.AdvanceCursor(&token)) continue;
    printf("%s\n", token.DebugString().c_str());
  }
  return 0;
}
