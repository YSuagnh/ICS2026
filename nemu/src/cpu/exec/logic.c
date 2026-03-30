#include "cpu/decode.h"
#include "cpu/exec.h"
#include "cpu/rtl.h"

#define width_mask(x) ((x) == 4 ? 0xffffffffu : ((1u << ((x) << 3)) - 1))

make_EHelper(test) {
  rtl_and(&t2, &id_dest->val, &id_src->val);

  rtl_update_ZFSF(&t2, id_dest->width);
  rtl_li(&t0, 0);
  rtl_set_CF(&t0);
  rtl_set_OF(&t0);

  print_asm_template2(test);
}

make_EHelper(and) {
  rtl_and(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);

  rtl_update_ZFSF(&t2, id_dest->width);
  rtl_li(&t0, 0);
  rtl_set_CF(&t0);
  rtl_set_OF(&t0);

  print_asm_template2(and);
}

make_EHelper(xor) {
  rtl_xor(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);

  rtl_update_ZFSF(&t2, id_dest->width);
  rtl_li(&t0, 0);
  rtl_set_CF(&t0);
  rtl_set_OF(&t0);

  print_asm_template2(xor);
}

make_EHelper(or) {
  rtl_or(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);

  rtl_update_ZFSF(&t2, id_dest->width);
  rtl_li(&t0, 0);
  rtl_set_CF(&t0);
  rtl_set_OF(&t0);
  print_asm_template2(or);
}

make_EHelper(sar) {
  // unnecessary to update CF and OF in NEMU
  rtl_sar(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->width);
  print_asm_template2(sar);
}

make_EHelper(shl) {
  // unnecessary to update CF and OF in NEMU
  rtl_shl(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->width);
  print_asm_template2(shl);
}

make_EHelper(shr) {
  // unnecessary to update CF and OF in NEMU
  rtl_shr(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->width);
  print_asm_template2(shr);
}

make_EHelper(rol) {
  uint32_t bits = id_dest->width << 3;
  uint32_t mask = width_mask(id_dest->width);
  uint32_t count = id_src->val & 0x1f;
  uint32_t val = id_dest->val & mask;

  count %= bits;
  if (count != 0) {
    val = ((val << count) | (val >> (bits - count))) & mask;
    rtl_li(&t0, val & 0x1);
    rtl_set_CF(&t0);
  }

  rtl_li(&t2, val);
  operand_write(id_dest, &t2);
  print_asm_template2(rol);
}

make_EHelper(ror) {
  uint32_t bits = id_dest->width << 3;
  uint32_t mask = width_mask(id_dest->width);
  uint32_t count = id_src->val & 0x1f;
  uint32_t val = id_dest->val & mask;

  count %= bits;
  if (count != 0) {
    val = ((val >> count) | (val << (bits - count))) & mask;
    rtl_li(&t0, (val >> (bits - 1)) & 0x1);
    rtl_set_CF(&t0);
  }

  rtl_li(&t2, val);
  operand_write(id_dest, &t2);
  print_asm_template2(ror);
}

make_EHelper(rcl) {
  uint32_t bits = id_dest->width << 3;
  uint32_t mask = width_mask(id_dest->width);
  uint32_t count = (id_src->val & 0x1f) % (bits + 1);
  uint32_t val = id_dest->val & mask;

  if (count != 0) {
    rtl_get_CF(&t1);
    uint32_t cf = t1 & 0x1;
    for (uint32_t i = 0; i < count; i++) {
      uint32_t new_cf = (val >> (bits - 1)) & 0x1;
      val = ((val << 1) & mask) | cf;
      cf = new_cf;
    }
    rtl_li(&t0, cf);
    rtl_set_CF(&t0);
  }

  rtl_li(&t2, val);
  operand_write(id_dest, &t2);
  print_asm_template2(rcl);
}

make_EHelper(rcr) {
  uint32_t bits = id_dest->width << 3;
  uint32_t mask = width_mask(id_dest->width);
  uint32_t count = (id_src->val & 0x1f) % (bits + 1);
  uint32_t val = id_dest->val & mask;

  if (count != 0) {
    rtl_get_CF(&t1);
    uint32_t cf = t1 & 0x1;
    for (uint32_t i = 0; i < count; i++) {
      uint32_t new_cf = val & 0x1;
      val = (val >> 1) | (cf << (bits - 1));
      val &= mask;
      cf = new_cf;
    }
    rtl_li(&t0, cf);
    rtl_set_CF(&t0);
  }

  rtl_li(&t2, val);
  operand_write(id_dest, &t2);
  print_asm_template2(rcr);
}

make_EHelper(setcc) {
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(subcode), id_dest->str);
}

make_EHelper(not) {
  rtl_mv(&t2, &id_dest->val);
  rtl_not(&t2);
  operand_write(id_dest, &t2);
  print_asm_template1(not);
}
