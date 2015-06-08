#include "multiboot/multiboot.h"
#include "vga/tinyvga.h"
#include "util/format.h"
#include "x86/interrupts/IDT.h"
#include "x86/interrupts/ISR.h"
#include "x86/interrupts/IRQ.h"
#include <stdint.h>
#include <stddef.h>

static tinyvga vga;
static IDT idt;
static ISR_table_t isrt;



const multiboot_memory_map_t* mmap_begin (const multiboot_info_t* info)
{
	return (const multiboot_memory_map_t*) (uint64_t) info->mmap_addr;
}

const multiboot_memory_map_t* mmap_end (const multiboot_info_t* info)
{
	return (const multiboot_memory_map_t*) (uint64_t) (info->mmap_addr + info->mmap_length);
}

const multiboot_memory_map_t* mmap_next (const multiboot_memory_map_t* map)
{
	uint32_t size = map->size + sizeof (map->size);
	const char* map_addr = (const char*) map;
	const char* next_addr = map_addr + size;
	return (const multiboot_memory_map_t*) next_addr;
}

void print_multiboot_memmap_entry (const multiboot_memory_map_t* map)
{
	static const char* typenames [] = {
		NULL,
		"AVAILABLE       ",
		"RESERVED        ",
		"ACPI_RECLAIMABLE",
		"NVS             ",
		"BADRAM          "
	};

	char buffer [17];
	vga_put (&vga, "  ");
	vga_put (&vga, typenames [map->type]);
	vga_put (&vga, " [0x");
	vga_put (&vga, format_uint (buffer, map->addr, 16, 16));
	vga_put (&vga, " - 0x");
	vga_put (&vga, format_uint (buffer, map->addr + map->len - 1, 16, 16));
	vga_putline (&vga, "]");
}

void print_multiboot_memmap (const multiboot_info_t* info)
{
	if (!(info->flags & MULTIBOOT_INFO_MEM_MAP))
		return;

	vga_putline (&vga, "Memory map:");
	uint64_t memsize = 0;
	uint64_t amemsize = 0;
	for (const multiboot_memory_map_t* map = mmap_begin (info);
	     map != mmap_end (info);
	     map = mmap_next (map))
	{
		print_multiboot_memmap_entry (map);
		memsize += map->len;
		if (map->type == MULTIBOOT_MEMORY_AVAILABLE)
			amemsize += map->len;
	}

	char buffer [20 + (20 - 1)/3 + 1];
	vga_put (&vga, "  Total ");
	vga_put (&vga, numsep (format_uint (buffer, memsize, 0, 10), ','));
	vga_put (&vga, " bytes (");
	vga_put (&vga, numsep (format_uint (buffer, amemsize, 0, 10), ','));
	vga_putline (&vga, " bytes available)");
}



void halt (void)
{
	__asm__ volatile (
		"cli;"
		"halt%=:"
		"hlt;"
		"jmp halt%="
		:
	);
}

void wait (void)
{
	__asm__ volatile (
		"sti;"
		"wait%=:"
		"hlt;"
		"jmp wait%="
		:
	);
}

static
void halt_ISR (INT_index interrupt, uint64_t error)
{
	char buffer [5];
	vga_put (&vga, "Interrupt: v=");
	vga_put (&vga, format_uint (buffer, interrupt, 2, 16));
	vga_put (&vga, " e=");
	vga_putline (&vga, format_uint (buffer, error, 4, 16));

	if (interrupt <= INT_SIMD_exception)
		halt ();
}

#include "kernel.h"


#include "memory/physmem.h"

void print_32b_backwards (uint32_t value)
{
	char buffer [33];
	format_uint (buffer, value, 32, 2);
	for (char *p = buffer, *q = buffer + 31; p < q; ++p, --q) {
		char tmp = *p;
		*p = *q;
		*q = tmp;
	}
	vga_put (&vga, buffer);
}

void print_allocator (physmem_allocator phy)
{
	uint64_t mem_size = (phy.bmp_end - phy.bmp_begin)*64*4096;
	char buffer [65];
	vga_put (&vga, "Physical allocator for 0x");
	vga_put (&vga, format_uint (buffer, (uintptr_t)phy.mem_base, 16, 16));
	vga_put (&vga, " - 0x");
	vga_put (&vga, format_uint (buffer, (uintptr_t)phy.mem_base + mem_size - 1, 16, 16));
	vga_putline (&vga, ":");

	for (uint64_t* block = phy.bmp_begin; block != phy.bmp_end; ++block) {
		vga_put (&vga, "  ");
		print_32b_backwards (*block & 0xFFFFFFFF);
		vga_put (&vga, " ");
		print_32b_backwards ((*block >> 32) & 0xFFFFFFFF);
		vga_putline (&vga, "");
	}
}

void kernel_main (multiboot_info_t* info,
                  __attribute__ ((unused)) multiboot_uint32_t magic)
{
	vga = vga_initialize ();
	vga_clear (&vga);
	vga_putline (&vga, "Success.");

	ISR_table_initialize (&isrt, &halt_ISR);
	IDT_initialize (&idt);
	IRQ_disable (IRQ_PIT);

	print_multiboot_memmap (info);

	char buffer [17];
	vga_put (&vga, "Kernel image start: 0x");
	vga_putline (&vga, format_uint (buffer, kernel_base, 16, 16));
	vga_put (&vga, "Kernel image end:   0x");
	vga_putline (&vga, format_uint (buffer, kernel_base + kernel_size, 16, 16));

	uint64_t allocbuf [4];
	physmem_allocator phy = physmem_make_allocator (allocbuf, 0, 4 * 64 * 4096);
	print_allocator (phy);

	for (int i = 0; i < 70; ++i) {
		if (!physmem_alloc (&phy).success) {
			vga_putline (&vga, "Unsuccessful allocation!");
			halt ();
		}
	}
	print_allocator (phy);

	int pages [] = {8, 6, 7, 5, 3, 0, 9};
	for (int i = 0; i < 7; ++i)
		physmem_free (&phy, (uint8_t*)(uintptr_t)(pages [i] * 4096));
	print_allocator (phy);

	for (int i = 0; i < 10; ++i) {
		if (!physmem_alloc (&phy).success) {
			vga_putline (&vga, "Unsuccessful allocation!");
			halt ();
		}
	}
	print_allocator (phy);

	wait ();
}
