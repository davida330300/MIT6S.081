#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"

int g(int x) {
  return x+3;
}

int f(int x) {
  return g(x);
}
