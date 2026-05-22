#include "am.h"
#include <x86.h>
#include <x86_64-linux-gnu/sys/types.h>

#define PG_ALIGN __attribute((aligned(PGSIZE)))

static PDE kpdirs[NR_PDE] PG_ALIGN;
static PTE kptabs[PMEM_SIZE / PGSIZE] PG_ALIGN;
static void* (*palloc_f)();
static void (*pfree_f)(void*);

_Area segments[] = {      // Kernel memory mappings
  {.start = (void*)0,          .end = (void*)PMEM_SIZE}
};

#define NR_KSEG_MAP (sizeof(segments) / sizeof(segments[0]))

void _pte_init(void* (*palloc)(), void (*pfree)(void*)) {
  palloc_f = palloc;
  pfree_f = pfree;

  int i;

  // make all PDEs invalid
  for (i = 0; i < NR_PDE; i ++) {
    kpdirs[i] = 0;
  }

  PTE *ptab = kptabs;
  for (i = 0; i < NR_KSEG_MAP; i ++) {
    uint32_t pdir_idx = (uintptr_t)segments[i].start / (PGSIZE * NR_PTE);
    uint32_t pdir_idx_end = (uintptr_t)segments[i].end / (PGSIZE * NR_PTE);
    for (; pdir_idx < pdir_idx_end; pdir_idx ++) {
      // fill PDE
      kpdirs[pdir_idx] = (uintptr_t)ptab | PTE_P;

      // fill PTE
      PTE pte = PGADDR(pdir_idx, 0, 0) | PTE_P;
      PTE pte_end = PGADDR(pdir_idx + 1, 0, 0) | PTE_P;
      for (; pte < pte_end; pte += PGSIZE) {
        *ptab = pte;
        ptab ++;
      }
    }
  }

  set_cr3(kpdirs);
  set_cr0(get_cr0() | CR0_PG);
}

void _protect(_Protect *p) {
  PDE *updir = (PDE*)(palloc_f());
  p->ptr = updir;
  // map kernel space
  for (int i = 0; i < NR_PDE; i ++) {
    updir[i] = kpdirs[i];
  }

  p->area.start = (void*)0x8000000;
  p->area.end = (void*)0xc0000000;
}

void _release(_Protect *p) {
}

void _switch(_Protect *p) {
  set_cr3(p->ptr);
}

void _map(_Protect *p, void *va, void *pa) {
  PDE *updir = (PDE*)(p->ptr);
  uint32_t pdir_idx = (uintptr_t)va >> 22;
  uint32_t pt_idx = ((uintptr_t)va >> 12) & 0x3ff;
  
  if (!(updir[pdir_idx] & PTE_P)) {
    PTE *ptab = (PTE*)palloc_f();
    updir[pdir_idx] = (uintptr_t)ptab | PTE_P;
  }
  
  PTE *ptab = (PTE*)(updir[pdir_idx] & ~0xfff);
  ptab[pt_idx] = (uintptr_t)pa | PTE_P;
}

void _unmap(_Protect *p, void *va) {
}

_RegSet *_umake(_Protect *p, _Area ustack, _Area kstack, void *entry, char *const argv[], char *const envp[]) {
  (void)p;
  (void)kstack;
  (void)argv;
  (void)envp;

  uintptr_t sp = (uintptr_t)ustack.end;

  // _start(argc=0, argv=NULL, envp=NULL)
  sp -= sizeof(uintptr_t);
  *(uintptr_t *)sp = 0; // envp
  sp -= sizeof(uintptr_t);
  *(uintptr_t *)sp = 0; // argv
  sp -= sizeof(uintptr_t);
  *(uintptr_t *)sp = 0; // argc
  sp -= sizeof(uintptr_t);
  *(uintptr_t *)sp = 0; // return address (unused)

  uintptr_t user_sp = sp;

  sp -= sizeof(_RegSet);
  _RegSet *tf = (_RegSet *)sp;
  *tf = (_RegSet){0};
  tf->eip = (uintptr_t)entry;
  tf->cs = 8;
  tf->eflags = FL_IF;
  tf->esp = user_sp - sizeof(uintptr_t) * 5;

  return tf;
}