#include "arch.h"
#include "common.h"
#include "memory.h"
#include <sys/types.h>

#define DEFAULT_ENTRY ((void *)0x8048000)

void ramdisk_write(const void *buf, off_t offset, size_t len);
void ramdisk_read(const void *buf, off_t offset, size_t len);
size_t get_ramdisk_size();

ssize_t fs_read(int fd, void *buf, int len);
ssize_t fs_write(int fd, void *buf, int len);
int fs_open(const char *pathname, int flags, int mode);
off_t fs_lseek(int fd, off_t offset, int whence);
int fs_close(int fd);
ssize_t fs_fsize(int fd);

uintptr_t loader(_Protect *as, const char *filename) {
  Log("Filename is %s", filename);
  int fd = fs_open(filename, 0, 0);
  int fsize = fs_fsize(fd);
  uint32_t vaddr = (uint32_t)DEFAULT_ENTRY;
  for(uint32_t p = 0; p < fsize; p += PGSIZE, vaddr += PGSIZE) {
    uint32_t paddr = (uint32_t)new_page();
    _map(as, (void*)vaddr, (void*)paddr);
    fs_read(fd, (void*)paddr, PGSIZE);
  }
  return (uintptr_t)DEFAULT_ENTRY;
}
