include ../toolchain.mk

CSOURCES := $(shell find -name '*.c')
COBJECTS := $(patsubst %.c,%.c.o,$(CSOURCES))
CDEPENDS := $(patsubst %.c,%.c.d,$(CSOURCES))

ASMSOURCES := $(shell find -name '*.s')
ASMOBJECTS := $(patsubst %.s,%.s.o,$(ASMSOURCES))

DEPENDS := $(CDEPENDS)
OBJECTS := $(COBJECTS) $(ASMOBJECTS)

.PHONY: all depends clean cleandeps

all: init.o
depends: $(DEPENDS)

clean:
	rm -f init.o $(OBJECTS)
cleandeps:
	rm -f $(DEPENDS)

init.o: init.ld $(OBJECTS)
	$(LD) $(LD32FLAGS) -r -T $< -o $@ $(OBJECTS)
	objcopy -G 'init' -O elf64-x86-64 $@

-include $(DEPENDS)

$(OBJECTS): Makefile

%.c.d: %.c
	$(CC) -I. -I.. -MM $< | sed 's/^\(.*\)\.o:/\1.c.d \1.c.o:/' > $@

%.c.o: %.c
	$(CC) $(C32FLAGS) -I.. -c -o $@ $<

%.s.o: %.s
	$(AS) $(AS32FLAGS) -o $@ $<
