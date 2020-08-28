#include <cstdio>
#include <cmath>

const char* itoa(int x)
{
  if (x == 0) return "0";
  char buff[64] = {}; 
  int i = 0;
  if (x < 0) {
    buff[i++] = '-';
    x *= -1;
  }
  int div = (int)pow(10, (int)log10(x));
  int mod = x + 1;
  while (div > 0) {
    buff[i++] =  (((x % mod) / div) + '0');
    mod = div;
    div /= 10;
  }
  return buff;
}

int main(int argc, char** argv)
{
  printf("%i == '%s'\n", 12345, itoa(12345));
  printf("%i == '%s'\n", 46, itoa(46));
  printf("%i == '%s'\n", 23, itoa(23));
  printf("%i == '%s'\n", 8912123, itoa(8912123));
  printf("%i == '%s'\n", 7, itoa(7));
  printf("%i == '%s'\n", 0, itoa(0));
  printf("%i == '%s'\n", 10003, itoa(10003));
  printf("%i == '%s'\n", -10003, itoa(-10003));
  return 0;
}
