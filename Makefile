OSNAME = "Unnamed kernel"

AS = as
override AS32FLAGS:=$(ASFLAGS) $(AS32FLAGS) -march=i686 --32
override AS64FLAGS:=$(ASFLAGS) $(AS64FLAGS) --64
CC = gcc
override CFLAGS:=$(CFLAGS) -I. -std=gnu99 -ffreestanding -Os -Wall -Wextra -Werror
override C32FLAGS:=$(CFLAGS) $(C32FLAGS) -march=i686 -m32
override C64FLAGS:=$(CFLAGS) $(C64FLAGS) -m64
LD = ld
override LDFLAGS:=$(LDFLAGS) -nostdlib
override LD32FLAGS:=$(LD32FLAGS) -march=i686 -melf_i386
override LD64FLAGS:=$(LD64FLAGS)

NOROMFLAG = -netdev user,id=hostnet0 -device virtio-net-pci,romfile=,netdev=hostnet0 # Kill iPXE option ROM
override QEMUFLAGS:=$(NOROMFLAG) $(QEMUFLAGS)

CSOURCES := $(shell find -name '*.c')
COBJECTS := $(patsubst %.c,%.c.o,$(CSOURCES))
CDEPENDS := $(patsubst %.c,%.c.d,$(CSOURCES))

ASMSOURCES := $(shell find -name '*.s')
ASMOBJECTS := $(patsubst %.s,%.s.o,$(ASMSOURCES))

DEPENDS := $(CDEPENDS)
OBJECTS := $(COBJECTS) $(ASMOBJECTS)

.PHONY: all depends test test-grub clean cleandeps

all: kernel.bin
depends: $(DEPENDS)

test: kernel.bin
	qemu-system-x86_64 -kernel $< $(QEMUFLAGS)
test-grub: kernel.iso
	qemu-system-x86_64 -cdrom $< $(QEMUFLAGS)

clean:
	rm -f kernel.iso kernel.bin $(OBJECTS)
cleandeps:
	rm -f $(DEPENDS)

kernel.bin: kernel.ld $(OBJECTS)
	$(LD) $(LD32FLAGS) -T $< -o $@ $(OBJECTS) --gc-sections #--print-gc-sections

kernel.iso: kernel.bin
	$(eval ISODIR := $(shell mktemp -d))
	mkdir -p $(ISODIR)
	mkdir -p $(ISODIR)/boot
	cp $< $(ISODIR)/boot/
	mkdir -p $(ISODIR)/boot/grub
	echo 'menuentry "'"$(OSNAME)"'" {\n multiboot /boot/'"$<"'\n}' > $(ISODIR)/boot/grub/grub.cfg
	pc-grub-mkrescue -o $@ $(ISODIR)
	rm -rf $(ISODIR)

-include $(DEPENDS)

$(OBJECTS): Makefile

%.c.d: %.c
	$(CC) -MM $< | sed 's/^\(.*\)\.o:/\1.c.d \1.c.o:/' > $@

%.c.o: %.c
	$(CC) $(C32FLAGS) -c -o $@ $<

%.s.o: %.s
	$(AS) $(AS32FLAGS) -o $@ $<
