#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */
  rtl_push(&cpu.flags);
  cpu.eflags.IF = false;
  rtl_push(&cpu.cs);
  rtl_li(&t0, ret_addr);
  rtl_push(&t0);
  if(decoding.is_operand_size_16) {
    decoding.jmp_eip = vaddr_read(cpu.idtr_16.base + 8 * NO, 2) | vaddr_read(cpu.idtr_16.base + 8 * NO + 6, 2) << 16;
  } else {
    decoding.jmp_eip = vaddr_read(cpu.idtr_32.base + 8 * NO, 2) | vaddr_read(cpu.idtr_32.base + 8 * NO + 6, 2) << 16;
  }
  decoding.is_jmp = 1;
}

void dev_raise_intr() {
  cpu.INTR = true;
}
