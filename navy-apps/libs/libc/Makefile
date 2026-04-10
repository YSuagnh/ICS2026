NAME = libc
SRCS = $(shell find src/ -name "*.c" -o -name "*.S" -o -name "*.cpp")

# newlib reent sources contain tentative global definitions (e.g. errno)
# that require -fcommon with modern GCC defaulting to -fno-common.
CFLAGS += -fcommon

include $(NAVY_HOME)/Makefile.lib
