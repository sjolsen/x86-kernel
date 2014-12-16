OSNAME = "Unnamed kernel"

include toolchain.mk

NOROMFLAG = -netdev user,id=hostnet0 -device virtio-net-pci,romfile=,netdev=hostnet0 # Kill iPXE option ROM
override QEMUFLAGS:=$(NOROMFLAG) $(QEMUFLAGS)

CSOURCES := $(shell find -name '*.c' -not -path './init/*')
COBJECTS := $(patsubst %.c,%.c.o,$(CSOURCES))
CDEPENDS := $(patsubst %.c,%.c.d,$(CSOURCES))

ASMSOURCES := $(shell find -name '*.s' -not -path './init/*')
ASMOBJECTS := $(patsubst %.s,%.s.o,$(ASMSOURCES))

DEPENDS := $(CDEPENDS)
OBJECTS := $(COBJECTS) $(ASMOBJECTS)

.PHONY: all depends test test-grub clean cleanall

kernel.bin32: kernel.bin64
	cp $< $@
	objcopy -O elf32-i386 $@

kernel.bin64: kernel.ld $(OBJECTS) init/init.o
	$(LD) $(LD64FLAGS) --nmagic -T $< -o $@ $(OBJECTS) init/init.o

all: kernel.iso kernel.bin64 kernel.bin32
depends: $(DEPENDS)

test: kernel.bin32
	qemu-system-x86_64 -kernel $< $(QEMUFLAGS)
test-grub: kernel.iso
	qemu-system-x86_64 -cdrom $< $(QEMUFLAGS)

clean:
	find '(' -name '*.d' -or -name '*.o' ')' -exec rm '{}' ';'
cleanall: clean
	rm -f kernel.iso kernel.bin32 kernel.bin64

kernel.iso: kernel.bin64
	$(eval ISODIR := $(shell mktemp -d))
	mkdir -p $(ISODIR)
	mkdir -p $(ISODIR)/boot
	cp $< $(ISODIR)/boot/
	mkdir -p $(ISODIR)/boot/grub
	echo 'menuentry "'"$(OSNAME)"'" {\n multiboot /boot/'"$<"'\n}' > $(ISODIR)/boot/grub/grub.cfg
	$(GRUB_MKRESCUE) -o $@ $(ISODIR)
	rm -rf $(ISODIR)

-include $(DEPENDS)

$(OBJECTS): Makefile

$(DEPENDS): %.c.d: %.c
	$(CC) -I. -MM $< | sed 's/^\(.*\)\.o:/\1.c.d \1.c.o:/' > $@

$(COBJECTS): %.c.o: %.c
	$(CC) $(C64FLAGS) -c -o $@ $<

$(ASMOBJECTS): %.s.o: %.s
	$(AS) $(AS64FLAGS) -o $@ $<

### Init subdirectory

INIT_CSOURCES := $(shell find ./init -name '*.c')
INIT_COBJECTS := $(patsubst %.c,%.c.o,$(INIT_CSOURCES))
INIT_CDEPENDS := $(patsubst %.c,%.c.d,$(INIT_CSOURCES))

INIT_ASMSOURCES := $(shell find ./init -name '*.s')
INIT_ASMOBJECTS := $(patsubst %.s,%.s.o,$(INIT_ASMSOURCES))

INIT_DEPENDS := $(INIT_CDEPENDS)
INIT_OBJECTS := $(INIT_COBJECTS) $(INIT_ASMOBJECTS)

init_depends: $(INIT_DEPENDS)

init/init.o: init/init.ld $(INIT_OBJECTS)
	$(LD) $(LD32FLAGS) -r -T $< -o $@ $(INIT_OBJECTS)
	objcopy -G 'init' -O elf64-x86-64 $@

-include $(INIT_DEPENDS)

$(INIT_OBJECTS): Makefile

$(INIT_DEPENDS): %.c.d: %.c
	$(CC) -I. -MM $< | sed 's/^\(.*\)\.o:/\1.c.d \1.c.o:/' > $@

$(INIT_COBJECTS): %.c.o: %.c
	$(CC) $(C32FLAGS) -c -o $@ $<

$(INIT_ASMOBJECTS): %.s.o: %.s
	$(AS) $(AS32FLAGS) -o $@ $<
