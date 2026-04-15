#include "fs.h"

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
  file_table[FD_FB].size = _screen.height * _screen.width * sizeof(uint32_t);
}

void ramdisk_read(void *buf, off_t offset, size_t len);
void ramdisk_write(const void *buf, off_t offset, size_t len);
void fb_write(const void *buf, off_t offset, size_t len);
void dispinfo_read(void *buf, off_t offset, size_t len);
size_t events_read(void *buf, size_t len);

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
  switch (fd) {
    case FD_DISPINFO :
      len = f->open_offset + len > f->size ? f->size - f->open_offset : len;
      dispinfo_read(buf, f->open_offset, len);
      f->open_offset += len;
      break;
    default:
      len = f->open_offset + len > f->size ? f->size - f->open_offset : len;
      ramdisk_read(buf, f->disk_offset + f->open_offset, len);
      f->open_offset += len;
      break;
    case FD_EVENTS :
      len = events_read(buf, len);
      break;
    case FD_STDERR :
    case FD_STDIN :
    case FD_STDOUT :
    case FD_FB :
      len = -1;
  }
  return len;
}

ssize_t fs_write(int fd, void *buf, int len) {
  if(!len) return 0;
  if(len < 0) return -1;
  Finfo *f = &file_table[fd];
  int cnt = -1;
  switch (fd) {
    case FD_STDOUT:
    case FD_STDERR:
      cnt = 0;
      while(cnt < len) {
        _putc(((char*)buf)[cnt++]);
      }
    case FD_STDIN:
    case FD_DISPINFO:
    case FD_EVENTS:
      len = cnt;
      break;
    case FD_FB :
      len = f->open_offset + len > f->size ? f->size - f->open_offset : len;
      fb_write(buf, f->open_offset, len);
      f->open_offset += len;
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