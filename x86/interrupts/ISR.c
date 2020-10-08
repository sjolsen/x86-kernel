#include "ISR.h"
#include "IRQ.h"
#include <stddef.h>

ISR_table_t* ISR_table;


// Extern functions

void ISR_entry (uint32_t interrupt, uint64_t error)
{
	/* According to https://wiki.osdev.org/8259_PIC#Spurious_IRQs, spurious
	 * interrupts result in the lowest-priority interrupt being signalled
	 * (LPT1 for the primary PIC, HDD2 for the secondary PIC). The interrupt
	 * handler should not be run and EOI should not be signalled for
	 * spurious interrupts; however the primary PIC still must receive an
	 * EOI for spurious interrupts proxied from the secondary PIC.
	 */
	if (interrupt == INT_LPT1 && !IRQ_in_service (IRQ_LPT1))
		return;
	if (interrupt == INT_HDD2 && !IRQ_in_service (IRQ_HDD2)) {
		IRQ_EOI_master ();
		return;
	}

	(*(*ISR_table) [interrupt]) (interrupt, error);

	if (INT_IRQ_MBASE <= interrupt && interrupt < INT_IRQ_SBASE)
		IRQ_EOI_master ();
	else if (INT_IRQ_SBASE <= interrupt)
		IRQ_EOI_slave ();
}

void null_ISR (__attribute__ ((unused)) INT_index interrupt,
               __attribute__ ((unused)) uint64_t error)
{
}

void set_ISR (INT_index interrupt, ISR_t isr)
{
	(*ISR_table) [(size_t)interrupt] = isr;
}

void ISR_table_initialize (ISR_table_t* table, ISR_t default_ISR)
{
	ISR_table = table;
	for (size_t i = 0; i < INT_LIMIT; ++i)
		set_ISR (i, default_ISR);
}
