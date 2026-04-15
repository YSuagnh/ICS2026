#include "fs.h"
#include <sys/types.h>

typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;
  off_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  // TODO: initialize the size of /dev/fb
}

void ramdisk_read(void *buf, off_t offset, size_t len);
void ramdisk_write(const void *buf, off_t offset, size_t len);

int fs_open(const char *pathname, int flags, int mode) {
  int len = strlen(pathname);
  for(int i = 0; i < NR_FILES; ++i) {
    if(!memcmp(pathname, file_table[i].name, len)) {
      file_table[i].open_offset = 0;
      return i;
    }
  }
  assert("should not reach");
  return 0;
}

ssize_t fs_read(int fd, void *buf, int len) {
  Finfo *f = &file_table[fd];
  len = f->open_offset + len > f->size ? f->size - f->open_offset : len;
  ramdisk_read(buf, f->disk_offset + f->open_offset, len);
  f->open_offset += len;
  return len;
}

ssize_t fs_write(int fd, void *buf, int len) {
  if(!len) return 0;
  if(len < 0) return -1;
  Finfo *f = &file_table[fd];
  int cnt = -1;
  switch (fd) {
    case 1:
    case 2:
      cnt = 0;
      while(cnt < len) {
        _putc(((char*)buf)[cnt++]);
      }
    case 0:
    case 3:
    case 4:
    case 5:
      len = cnt;
      break;
    default:
      len = f->open_offset + len > f->size ? f->size - f->open_offset : len;
      ramdisk_write(buf, f->disk_offset + f->open_offset, len);
      f->open_offset += len;
      break;
  }
  return len;
}

off_t fs_lseek(int fd, off_t offset, int whence) {
  Finfo *f = &file_table[fd];
  switch (whence) {
    case SEEK_SET :
      f->open_offset = offset;
      break;
    case SEEK_CUR :
      f->open_offset += offset;
      break;
    case SEEK_END :
      f->open_offset = f->size + offset;
      break;
  }
  if(f->open_offset < 0) f->open_offset = 0;
  if(f->open_offset > f->size) f->open_offset = f->size;
  return f->size;
}

int fs_close(int fd) {
  return 0;
}

ssize_t fs_fsize(int fd) {
  return file_table[fd].size;
}