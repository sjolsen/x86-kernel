#include <stdint.h>
#include <stddef.h>

static
void halt (void)
{
		__asm__ volatile (
		".halt:"
			"cli;"
			"hlt;"
			"jmp .halt"
		);
}

void kernel_main (void)
{
	uint64_t tmp = 0;
	__asm__ volatile (
		"movq $0x0763076307750753, %0;"
		"movq %0, 0(%1);"
		"movq $0x072e077307730765, %0;"
		"movq %0, 8(%1)"
		: "+r" (tmp)
		: "r" (0xB8000)
	);
	halt ();
}
