#include <cstdio>

static int kFib[100] = {};

int fib_1(int n) {
  if (n == 0) return 0;
  if (n == 1) return 1;
  return fib_1(n - 1) + fib_1(n - 2);
}

int fib_2(int n) {
  if (n <= 1) return n;
  int t1 = 0, t2 = 1;
  for (int i = 2; i <= n; ++i) {
    int t = t2;
    t2 = t2 + t1;
    t1 = t;
  }
  return t2;
}

int fib_3(int n) {
  if (n <= 1) return n;
  if (kFib[n]) return kFib[n];
  return fib_3(n - 1) + fib_3(n - 2);
}

int main(int argc, char** argv) {
  for (int i = 0; i < 10; ++i) {
    printf("fib_1(%i): %i\n", i, fib_1(i));
  }

  for (int i = 0; i < 10; ++i) {
    printf("fib_2(%i): %i\n", i, fib_2(i));
  }

  for (int i = 0; i < 10; ++i) {
    printf("fib_3(%i): %i\n", i, fib_3(i));
  }
  return 0;
}
