BUILDDIR?=_build

ENSUREDIR = if [ -n "$(@D)" -a ! -d "$(@D)" ]; then mkdir -p "$(@D)"; fi

AS = as
override AS32FLAGS:=$(ASFLAGS) $(AS32FLAGS) -march=i686 --32
override AS64FLAGS:=$(ASFLAGS) $(AS64FLAGS) --64

CC = gcc
override CFLAGS:=$(CFLAGS) -I. -std=gnu99 -ffreestanding -fno-asynchronous-unwind-tables -fno-pie -mno-sse -Os -g -Wall -Wextra -Werror
override C32FLAGS:=$(CFLAGS) $(C32FLAGS) -march=i686 -m32
override C64FLAGS:=$(CFLAGS) $(C64FLAGS) -m64 -mcmodel=kernel -mno-red-zone

LD = ld
override LDFLAGS:=$(LDFLAGS) -nostdlib
override LD32FLAGS:=$(LDFLAGS) $(LD32FLAGS) -march=i686 -melf_i386
override LD64FLAGS:=$(LDFLAGS) $(LD64FLAGS)

GRUB_MKRESCUE = grub-mkrescue
