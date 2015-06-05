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

	wait ();
}
