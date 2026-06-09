#include "FLOAT.h"
#include <stdint.h>

FLOAT F_mul_F(FLOAT a, FLOAT b) {
  int sign = 1;
  if ((a < 0) && (b >= 0)) sign = -1;
  if ((a >= 0) && (b < 0)) sign = -1;

  unsigned int ua = (a < 0) ? (unsigned int)(-(int)a) : (unsigned int)a;
  unsigned int ub = (b < 0) ? (unsigned int)(-(int)b) : (unsigned int)b;

  unsigned int lo = (ua & 0xFFFF) * (ub & 0xFFFF);
  unsigned int hi = (ua >> 16) * (ub >> 16);
  unsigned int mid1 = (ua >> 16) * (ub & 0xFFFF);
  unsigned int mid2 = (ua & 0xFFFF) * (ub >> 16);

  unsigned int carry = ((lo >> 16) + (mid1 & 0xFFFF) + (mid2 & 0xFFFF)) >> 16;
  unsigned int result_hi = hi + (mid1 >> 16) + (mid2 >> 16) + carry;

  unsigned int product = (result_hi << 16) | ((lo >> 16) + (mid1 & 0xFFFF) + (mid2 & 0xFFFF));

  if (product > 0x7FFFFFFF) {
    product = 0x7FFFFFFF;
  }

  if (sign == -1) {
    return -(FLOAT)product;
  }
  return (FLOAT)product;
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
  if (b == 0) {
    if (a == 0) return 0;
    else if (a > 0) return (FLOAT)0x7FFFFFFF;
    else return (FLOAT)0x80000000;
  }

  int sign = 1;
  if ((a < 0) && (b >= 0)) sign = -1;
  if ((a >= 0) && (b < 0)) sign = -1;

  unsigned int ua = (a < 0) ? (unsigned int)(-(int)a) : (unsigned int)a;
  unsigned int ub = (b < 0) ? (unsigned int)(-(int)b) : (unsigned int)b;

  unsigned int dividend_hi = 0;
  unsigned int dividend_lo = ua;
  unsigned int divisor = ub;
  unsigned int quotient = 0;

  int i;
  for (i = 0; i < 16; i++) {
    dividend_hi = (dividend_hi << 1) | ((dividend_lo >> 31) & 1);
    dividend_lo = dividend_lo << 1;
  }

  for (i = 0; i < 16; i++) {
    dividend_hi = dividend_hi << 1;
    dividend_lo = dividend_lo << 1;

    if (dividend_hi >= divisor) {
      dividend_hi = dividend_hi - divisor;
      quotient = quotient | (1U << (15 - i));
    }
  }

  if (quotient > 0x7FFFFFFF) {
    quotient = 0x7FFFFFFF;
  }

  if (sign == -1) {
    return -(FLOAT)quotient;
  }
  return (FLOAT)quotient;
}

FLOAT f2F(float a) {
  int *ptr = (int *)&a;
  unsigned int bits = *(unsigned int *)ptr;

  unsigned int sign_bit = (bits >> 31) & 1;
  unsigned int exp = (bits >> 23) & 0xFF;
  unsigned int mant = bits & 0x7FFFFF;

  if (exp == 0xFF) return 0;

  if (exp == 0) return 0;

  unsigned int significand = (1 << 23) | mant;
  int shift = (int)exp - 134;

  unsigned int abs_val;

  if (shift >= 0) {
    if (shift > 31) {
      abs_val = 0x7FFFFFFF;
    } else {
      abs_val = significand << shift;
      if (abs_val > 0x7FFFFFFF) {
        abs_val = 0x7FFFFFFF;
      }
    }
  } else {
    int rshift = -shift;
    if (rshift >= 24) {
      unsigned int half = 1U << (rshift - 1);
      abs_val = (significand >= half) ? 1 : 0;
    } else {
      unsigned int q = significand >> rshift;
      unsigned int r = significand & ((1U << rshift) - 1);
      unsigned int half = 1U << (rshift - 1);
      if (r >= half) {
        q = q + 1;
      }
      abs_val = q;
    }
  }

  if (sign_bit) {
    if (abs_val >= 0x80000000U) {
      return (FLOAT)(-2147483647 - 1);
    } else {
      return -(FLOAT)abs_val;
    }
  } else {
    if (abs_val > 0x7FFFFFFF) {
      return (FLOAT)0x7FFFFFFF;
    } else {
      return (FLOAT)abs_val;
    }
  }
}

FLOAT Fabs(FLOAT a) {
  if (a >= 0) return a;
  if (a == (FLOAT)0x80000000) return (FLOAT)0x7FFFFFFF;
  return -a;
}

/* Functions below are already implemented */

FLOAT Fsqrt(FLOAT x) {
  FLOAT dt, t = int2F(2);

  do {
    dt = F_div_int((F_div_F(x, t) - t), 2);
    t += dt;
  } while(Fabs(dt) > f2F(1e-4));

  return t;
}

FLOAT Fpow(FLOAT x, FLOAT y) {
  /* we only compute x^0.333 */
  FLOAT t2, dt, t = int2F(2);

  do {
    t2 = F_mul_F(t, t);
    dt = (F_div_F(x, t2) - t) / 3;
    t += dt;
  } while(Fabs(dt) > f2F(1e-4));

  return t;
}