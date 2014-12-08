#include "multiboot.h"

#define FLAGS (MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO)

extern
__attribute__ ((section (".multiboot")))
const struct multiboot_header kernel_header;

__attribute__ ((aligned (MULTIBOOT_HEADER_ALIGN)))
const struct multiboot_header kernel_header = {
	.magic         = MULTIBOOT_HEADER_MAGIC,
	.flags         = FLAGS,
	.checksum      = -(MULTIBOOT_HEADER_MAGIC + FLAGS),
	.header_addr   = 0,
	.load_addr     = 0,
	.load_end_addr = 0,
	.bss_end_addr  = 0,
	.entry_addr    = 0,
	.mode_type     = 0,
	.width         = 0,
	.height        = 0,
	.depth         = 0,
};
