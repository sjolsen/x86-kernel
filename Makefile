include toolchain.mk

default: all

### 32-bit initialization code

INIT_CSOURCES := $(shell find ./init -name '*.c' | xargs realpath --relative-to=.)
INIT_COBJECTS := $(patsubst %.c,$(BUILDDIR)/%.c.o,$(INIT_CSOURCES))
INIT_CDEPENDS := $(patsubst %.c,$(BUILDDIR)/%.c.d,$(INIT_CSOURCES))

INIT_ASMSOURCES := $(shell find ./init -name '*.s' | xargs realpath --relative-to=.)
INIT_ASMOBJECTS := $(patsubst %.s,$(BUILDDIR)/%.s.o,$(INIT_ASMSOURCES))

INIT_DEPENDS := $(INIT_CDEPENDS)
INIT_OBJECTS := $(INIT_COBJECTS) $(INIT_ASMOBJECTS)

init_depends: $(INIT_DEPENDS)

$(BUILDDIR)/init/init.o: init/init.ld $(INIT_OBJECTS)
	@$(ENSUREDIR)
	@printf "LD\t$@\n"
	@$(LD) $(LD32FLAGS) -r -T $< -o $@ $(INIT_OBJECTS)
	@objcopy -G 'init' -O elf64-x86-64 $@

-include $(INIT_DEPENDS)

$(INIT_OBJECTS): Makefile

$(INIT_DEPENDS): $(BUILDDIR)/%.c.d: %.c
	@$(ENSUREDIR)
	@$(CC) -I. -MM $< | sed 's/^\(.*\)\.o:/\1.c.d \1.c.o:/' > $@

$(INIT_COBJECTS): $(BUILDDIR)/%.c.o: %.c
	@$(ENSUREDIR)
	@printf "CC\t$<\n"
	@$(CC) $(C32FLAGS) -c -o $@ $<

$(INIT_ASMOBJECTS): $(BUILDDIR)/%.s.o: %.s
	@$(ENSUREDIR)
	@printf "AS\t$<\n"
	@$(AS) $(AS32FLAGS) -o $@ $<

### 64-bit C/assembly sources

CSOURCES := $(shell find -name '*.c' -not -path './init/*' | xargs realpath --relative-to=.)
COBJECTS := $(patsubst %.c,$(BUILDDIR)/%.c.o,$(CSOURCES))
CDEPENDS := $(patsubst %.c,$(BUILDDIR)/%.c.d,$(CSOURCES))

ASMSOURCES := $(shell find -name '*.s' -not -path './init/*' | xargs realpath --relative-to=.)
ASMOBJECTS := $(patsubst %.s,$(BUILDDIR)/%.s.o,$(ASMSOURCES))

DEPENDS := $(CDEPENDS)
OBJECTS := $(COBJECTS) $(ASMOBJECTS)

-include $(DEPENDS)

$(OBJECTS): Makefile

$(DEPENDS): $(BUILDDIR)/%.c.d: %.c
	@$(ENSUREDIR)
	@$(CC) -I. -MM $< | sed 's/^\(.*\)\.o:/\1.c.d \1.c.o:/' > $@

$(COBJECTS): $(BUILDDIR)/%.c.o: %.c
	@$(ENSUREDIR)
	@printf "CC\t$<\n"
	@$(CC) $(C64FLAGS) -c -o $@ $<

$(ASMOBJECTS): $(BUILDDIR)/%.s.o: %.s
	@$(ENSUREDIR)
	@printf "AS\t$<\n"
	@$(AS) $(AS64FLAGS) -o $@ $<

### Kernel images

KERNELS := scanmem

KIMAGES := $(foreach k,$(KERNELS),$(BUILDDIR)/$(k).elf)
K64IMAGES := $(patsubst %.elf,%.64.elf,$(KIMAGES))
KMAINS := $(patsubst %.elf,%.c.o,$(KIMAGES))
NONMAINOBJECTS := $(filter-out $(KMAINS),$(OBJECTS))

$(K64IMAGES):$(BUILDDIR)/%.64.elf: kernel.ld $(NONMAINOBJECTS) $(BUILDDIR)/%.c.o $(BUILDDIR)/init/init.o
	@$(ENSUREDIR)
	@printf "LD\t$@\n"
	@$(LD) $(LD64FLAGS) --nmagic -T $< -o $@ $(NONMAINOBJECTS) $(BUILDDIR)/$*.c.o $(BUILDDIR)/init/init.o

$(KIMAGES):%.elf: %.64.elf
	@$(ENSUREDIR)
	@printf "OBJCOPY\t$@\n"
	@objcopy -O elf32-i386 $< $@

### Meta targets

NOROMFLAG = -netdev user,id=hostnet0 -device virtio-net-pci,romfile=,netdev=hostnet0 # Kill iPXE option ROM
override QEMUFLAGS:=$(NOROMFLAG) $(QEMUFLAGS)

ALLIMAGES := $(BUILDDIR)/grub.iso $(KIMAGES)

.PHONY: all init_depends depends test test-grub clean cleanall

all: $(ALLIMAGES)
depends: $(DEPENDS)

test: $(BUILDDIR)/scanmem.elf
	qemu-system-x86_64 -kernel $< $(QEMUFLAGS)
test-grub: $(BUILDDIR)/grub.iso
	qemu-system-x86_64 -cdrom $< $(QEMUFLAGS)

clean:
	@find $(BUILDDIR) -type f $(foreach o,$(ALLIMAGES),-not -path $(o)) -exec rm '{}' ';'
cleanall:
	@[ -n "$(BUILDDIR)" ] || exit 1
	@rm -rf $(BUILDDIR)

$(BUILDDIR)/grub.iso: $(KIMAGES)
	@$(ENSUREDIR)
	@printf "GRUBISO\t$(@)\n"
	$(eval ISODIR := $@.dir)
	@rm -rf $(ISODIR)
	@mkdir $(ISODIR)
	@mkdir $(ISODIR)/boot
	@mkdir $(ISODIR)/boot/grub
	@cp $^ $(ISODIR)/boot/
	@for k in $(KERNELS); do \
		echo 'menuentry "'"$$k"'" {\nmultiboot /boot/'"$$k.elf"'\n}'; \
	done > $(ISODIR)/boot/grub/grub.cfg
	@if ! $(GRUB_MKRESCUE) -o $@ $(ISODIR) >$@.log 2>&1; then \
		RET=$$?; cat $@.log; exit $$RET; \
	fi
