#include "cpu/rtl.h"

#define Eflags cpu.eflags
/* Condition Code */

void rtl_setcc(rtlreg_t* dest, uint8_t subcode) {
  bool invert = subcode & 0x1;
  enum {
    CC_O, CC_NO, CC_B,  CC_NB,
    CC_E, CC_NE, CC_BE, CC_NBE,
    CC_S, CC_NS, CC_P,  CC_NP,
    CC_L, CC_NL, CC_LE, CC_NLE
  };

  // TODO: Query EFLAGS to determine whether the condition code is satisfied.
  // dest <- ( cc is satisfied ? 1 : 0)
  switch (subcode & 0xe) {
    case CC_O:
      *dest = Eflags.OF;
      break;
    case CC_B:
      *dest = Eflags.CF;
      break;
    case CC_E:
      *dest = Eflags.ZF;
      break;
    case CC_BE:
      *dest = Eflags.CF || Eflags.ZF;
      break;
    case CC_S:
      *dest = Eflags.SF;
      break;
    case CC_L:
      *dest = Eflags.SF != Eflags.OF;
      break;
    case CC_LE:
      *dest = Eflags.SF != Eflags.OF || Eflags.ZF;
      break;
    default: panic("should not reach here");
    case CC_P: panic("n86 does not have PF");
  }

  if (invert) {
    rtl_xori(dest, dest, 0x1);
  }
}
