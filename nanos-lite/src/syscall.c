#include "common.h"
#include "syscall.h"

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);
  switch (a[0]) {
    case SYS_none :
      r->eax = 1;
      break;
    case SYS_write :
      r->eax = 0;
      switch (a[1]) {
        case 1 :
        case 2 :
          while (r->eax < a[3]) {
            _putc(((char *)a[2])[r->eax]);
            ++r->eax;
          }
          break;
        default :
          r->eax = -1;
          break;
      }
      break;
    case SYS_exit :
      _halt(a[1]);
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return r;
}
