#include "am.h"
#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t len) {
  int key = _read_key();
  bool down = false;
  if (key & 0x8000) {
    key ^= 0x8000;
    down = true;
  }
  if (key != _KEY_NONE) {
    snprintf(buf, len, "%s %s\n", down ? "kd" : "ku", keyname[key]);
  } else {
    snprintf(buf, len, "t %d\n", _uptime());
  }
  return strlen((char *)buf);
}

static char dispinfo[128] __attribute__((used));

void dispinfo_read(void *buf, off_t offset, size_t len) {
  memcpy(buf, dispinfo + offset, len);
  return;
}

void fb_write(const void *buf, off_t offset, size_t len) {
  const uint32_t *pixels = (const uint32_t *)buf;
  int start_pixel = offset / sizeof(uint32_t);
  int x = start_pixel % _screen.width;
  int y = start_pixel / _screen.width;
  int nr_pixel = len / sizeof(uint32_t);

  while (nr_pixel > 0 && y < _screen.height) {
    int w = _screen.width - x;
    if (w > nr_pixel) w = nr_pixel;
    _draw_rect(pixels, x, y, w, 1);
    pixels += w;
    nr_pixel -= w;
    x = 0;
    y ++;
  }
}

void init_device() {
  _ioe_init();
  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  snprintf(dispinfo, sizeof(dispinfo), "WIDTH:%d\nHEIGHT:%d\n", _screen.width, _screen.height);
}
