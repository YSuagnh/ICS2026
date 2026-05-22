#include <stdint.h>
#include <unistd.h>

#define SYS_none 0
extern int _syscall_(int, intptr_t, intptr_t, intptr_t);

int main() {
  // this system call will trap into OS but do nothing
  write(1, "hello world!\n", 13);
  int r = _syscall_(SYS_none, 0, 0, 0);
  return (r == 1 ? 0 : 1);
}
