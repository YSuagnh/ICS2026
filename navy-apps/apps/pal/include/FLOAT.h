#ifndef __FLOAT_H__
#define __FLOAT_H__

#include "assert.h"

typedef int FLOAT;
#define pow2_16 0x10000
#define lowbit 0xffff
#define highbit 0x7fff0000

static inline int F2int(FLOAT a) {
  return a >> 16;
}

static inline FLOAT int2F(int a) {
  return a << 16;
}

static inline FLOAT F_mul_int(FLOAT a, int b) {
  return a * b;
}

static inline FLOAT F_div_int(FLOAT a, int b) {
  return a / b;
}

FLOAT f2F(float);
FLOAT F_mul_F(FLOAT, FLOAT);
FLOAT F_div_F(FLOAT, FLOAT);
FLOAT Fabs(FLOAT);
FLOAT Fsqrt(FLOAT);
FLOAT Fpow(FLOAT, FLOAT);

#endif
