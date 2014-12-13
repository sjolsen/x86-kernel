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

.PHONY: all depends test test-grub clean cleandeps

all: kernel.bin
depends: $(DEPENDS)

test: kernel.bin
	qemu-system-x86_64 -kernel $< $(QEMUFLAGS)
test-grub: kernel.iso
	qemu-system-x86_64 -cdrom $< $(QEMUFLAGS)

clean:
	rm -f kernel.iso kernel.bin $(OBJECTS)
	make -C init clean
cleandeps:
	rm -f $(DEPENDS)
	make -C init cleandeps

kernel.bin: kernel.ld $(OBJECTS) init/init.o
	$(LD) $(LD64FLAGS) --nmagic -T $< -o $@ $(OBJECTS) init/init.o --gc-sections #--print-gc-sections

kernel.iso: kernel.bin
	$(eval ISODIR := $(shell mktemp -d))
	mkdir -p $(ISODIR)
	mkdir -p $(ISODIR)/boot
	cp $< $(ISODIR)/boot/
	mkdir -p $(ISODIR)/boot/grub
	echo 'menuentry "'"$(OSNAME)"'" {\n multiboot /boot/'"$<"'\n}' > $(ISODIR)/boot/grub/grub.cfg
	grub-mkrescue -o $@ $(ISODIR)
	rm -rf $(ISODIR)

.PHONY: init/init.o
init/init.o:
	$(MAKE) -C ./init

-include $(DEPENDS)

$(OBJECTS): Makefile

%.c.d: %.c
	$(CC) -I. -MM $< | sed 's/^\(.*\)\.o:/\1.c.d \1.c.o:/' > $@

%.c.o: %.c
	$(CC) $(C64FLAGS) -c -o $@ $<

%.s.o: %.s
	$(AS) $(AS64FLAGS) -o $@ $<
