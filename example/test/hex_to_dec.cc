#include <cstdio>
#include <cstring>
#include <cmath>

int hex_dig(char dig) {
  switch (dig) {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    case 'A': return 10;
    case 'B': return 11;
    case 'C': return 12;
    case 'D': return 13;
    case 'E': return 14;
    case 'F': return 15;
  }
}

int to_decimal(const char* hex) {
  int res = 0;
  int len = strlen(hex);
  int j = 0;
  for (int i = len - 1; i >= 0; --i) {
    res += hex_dig(hex[i]) * (int)pow(16, j++);
  }
  return res;
}

void check(const char* hex) {
  printf("hex: %s -> dec: %i\n", hex, to_decimal(hex));
}

int main(int argc, char** argv) {
  check("AD45"); 
  return 0;
}
