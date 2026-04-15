#include "cpu/decode.h"
#include "cpu/exec.h"
#include "cpu/reg.h"
#include "cpu/rtl.h"
#include "debug.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
  rtl_push(&(id_dest->val));
  print_asm_template1(push);
}

make_EHelper(pop) {
  rtl_pop(&t0);
  operand_write(id_dest, &t0);
  print_asm_template1(pop);
}

make_EHelper(pusha) {
  rtl_mv(&t1, &cpu.esp);
  for(int i = 0; i < 8; ++i) {
    if(i == 4) rtl_push(&t1);
    else rtl_push(&cpu.gpr[i]._32);
  }
}

make_EHelper(popa) {
  for(int i = 7; ~i; --i) {
    if(i == 4) rtl_pop(&t1);
    else rtl_pop(&cpu.gpr[i]._32);
  }
  rtl_mv(&cpu.esp, &t1);
  print_asm("popa");
}

make_EHelper(leave) {
  int width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_lr(&t0, R_EBP, width);
  rtl_sr(R_ESP, width, &t0);

  rtl_lm(&t1, &t0, width);
  rtl_addi(&t0, &t0, width);
  rtl_sr(R_ESP, width, &t0);
  rtl_sr(R_EBP, width, &t1);
  
  print_asm("leave");
}

make_EHelper(cltd) {
  if (decoding.is_operand_size_16) {
    rtl_lr_w(&t0, R_AX);
    rtl_sext(&t1, &t0, 2);
    rtl_sr_l(R_EAX, &t1);
  }
  else {
    rtl_lr_l(&t0, R_EAX);
    rtl_msb(&t1, &t0, 4);
    rtl_sub(&t2, &tzero, &t1);
    rtl_sr_l(R_EDX, &t2);
  }

  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  if (decoding.is_operand_size_16) {
    rtl_lr_w(&t0, R_AX);
    rtl_sext(&t1, &t0, 2);
    rtl_sr_l(R_EAX, &t1);
  }
  else {
    rtl_lr_b(&t0, R_AL);
    rtl_sext(&t1, &t0, 1);
    rtl_sr_w(R_AX, &t1);
  }

  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_sext(&t2, &id_src->val, id_src->width);
  operand_write(id_dest, &t2);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  rtl_li(&t2, id_src->addr);
  operand_write(id_dest, &t2);
  print_asm_template2(lea);
}

make_EHelper(movs) {
  rtl_lm(&t0, &cpu.esi, id_dest->width);
  rtl_sm(&cpu.edi, id_dest->width, &t0);
  
  if (cpu.eflags.DF == 0) {
    rtl_addi(&cpu.esi, &cpu.esi, id_dest->width);
    rtl_addi(&cpu.edi, &cpu.edi, id_dest->width);
  } else {
    rtl_subi(&cpu.esi, &cpu.esi, id_dest->width);
    rtl_subi(&cpu.edi, &cpu.edi, id_dest->width);
  }

  print_asm_template1(movs);
}
