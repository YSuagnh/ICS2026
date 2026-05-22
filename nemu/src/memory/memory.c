#include "common.h"
#include "debug.h"
#include "device/mmio.h"
#include "nemu.h"
#include <assert.h>
#include <stdint.h>

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
  int id = is_mmio(addr);
  if(~id) return mmio_read(addr, len, id);
  else return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  int id = is_mmio(addr);
  if(~id) mmio_write(addr, len, data, id);
  else memcpy(guest_to_host(addr), &data, len);
}

paddr_t page_translate(vaddr_t vaddr) {
  if(cpu.CR0 & (1u<<31)) {
    uint32_t PDE = paddr_read((cpu.CR3 & ~0xfff) | ((vaddr >> 22) << 2), 4);
    assert(PDE & 1);
    uint32_t PTE = paddr_read((PDE & (~0xfff)) | (((vaddr >> 12) & 0x3ff) << 2), 4);
    assert(PTE & 1);
    paddr_t paddr = (PTE & (~0xfff)) | (vaddr & 0xfff);
    return paddr;
  } else {
    return vaddr;
  }
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  if ((addr & 0xfff) + len > 0x1000) {
    int len1 = 0x1000 - (addr & 0xfff);
    int len2 = len - len1;
    uint32_t data1 = vaddr_read(addr, len1);
    uint32_t data2 = vaddr_read(addr + len1, len2);
    return data1 | (data2 << (len1 << 3));
  } else {
    paddr_t paddr = page_translate(addr);
    return paddr_read(paddr, len);
  }
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  if ((addr & 0xfff) + len > 0x1000) {
    int len1 = 0x1000 - (addr & 0xfff);
    int len2 = len - len1;
    vaddr_write(addr, len1, data & (~0u >> ((4 - len1) << 3)));
    vaddr_write(addr + len1, len2, data >> (len1 << 3));
  } else {
    paddr_t paddr = page_translate(addr);
    paddr_write(paddr, len, data);
  }
}
