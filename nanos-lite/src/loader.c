#include "common.h"
#include <sys/types.h>

#define DEFAULT_ENTRY ((void *)0x4000000)

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
  Log("Filename is %s\n", filename);
  int fd = fs_open(filename, 0, 0);
  fs_read(fd, DEFAULT_ENTRY, fs_fsize(fd));
  return (uintptr_t)DEFAULT_ENTRY;
}
