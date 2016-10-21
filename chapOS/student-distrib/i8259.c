/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
uint8_t master_mask = 0xff; /* IRQs 0-7 */
uint8_t slave_mask = 0xff; /* IRQs 8-15 */


/* Initialize the 8259 PIC */
void
i8259_init(void)
{

	/* Both PICs are initially configured to mask all interrupts */


	/* Initalization control word 1. Tells PIC it's being initialized. */
	outb(ICW1, MASTER_8259_PORT);
	outb(ICW1, SLAVE_8259_PORT);


	/*** Remaining ICW's are written to second ports-- 0x21 and 0xA1. ***/


	/* Initialization control word 2. This control word is used to map the 
	 * base address of the IVT of which the PICS are to use. */
	outb(ICW2_MASTER, MASTER_8259_PORT_2);
	outb(ICW2_SLAVE, SLAVE_8259_PORT_2);


	/* Initialization control word 3. The specific IR pin used in the master/slave 
	 * relationship is specified by this word. */
	outb(ICW3_MASTER, MASTER_8259_PORT_2); //specifies which irq connected to slave pic
	outb(ICW3_SLAVE, SLAVE_8259_PORT_2); //irq number the master pic uses to connect to


	/* Initialization control word 4. This enables the PICs for 8086 mode. */
	outb(ICW4, MASTER_8259_PORT_2);
	outb(ICW4, SLAVE_8259_PORT_2);



	enable_irq(SLAVE_IRQ);
	// here must enable (unmask interrupts)
}

/* Enable (unmask) the specified IRQ */
void
enable_irq(uint32_t irq_num)
{
	/* The OCW1 goes to the register at Base + 1--ie. 0x21 and 0xA1. */
	 
	/*
	* Bits set in OCW1 mask/disable irqs. Thus must invert bits to 
	* instead enable irqs.
	*/
	uint8_t single_bit_mask = 0x01;
	uint32_t offset;
	/* First check if irq_num is out of bounds */
	if( irq_num > 15 || irq_num < 0)
		return;


	/* Master irq-- 0-7 */
	if(irq_num <= 7 && irq_num >= 0)
	{
		/* Bit that is not set corresponds to irq to be enabled (1111 1110)*/
		
		single_bit_mask = single_bit_mask << irq_num;
		single_bit_mask = ~single_bit_mask;

		/* Use AND so anything that is already enabled stays enabled */
		/* So a bit that's disabled (1) resolves to enabled (0) */
		master_mask = master_mask & single_bit_mask;

		outb( master_mask, MASTER_8259_PORT_2);
		return;
	}


	/* Slave irq-- 8-15 */
	if(irq_num >= 8 && irq_num <= 15)
	{
		/* Minus 8 to recalculate offset starting from irq 8 in slave pic */
		offset = irq_num - 8;
		single_bit_mask = single_bit_mask << offset;
		single_bit_mask = ~single_bit_mask;

		/* Use AND so anything that is already enabled stays enabled */
		/* So a bit that's disabled (1) resolves to enabled (0) */
		slave_mask = slave_mask & single_bit_mask;

		outb( slave_mask, SLAVE_8259_PORT_2);
		return;
	}

}

/* Disable (mask) the specified IRQ */
void
disable_irq(uint32_t irq_num)
{
	/* 
	 * Note: IRQ 2 is connected to PIC2, thus if it is 
	 * masked, then actually disabling IRQ's 8 to 15.
	 *
	 * The OCW1 goes to the register at Base + 1--ie. 0x21 and 0xA1.
	 * Bits set in OCW1 mask/disable irqs. 
	 */

	uint8_t single_bit_mask = 0x01; 
	uint32_t offset;
	/* First check if irq_num is out of bounds */
	if(irq_num > 15 || irq_num < 0)
		return;



	/* Master irq-- 0-7 */
	if(irq_num <= 7 && irq_num >= 0)
	{
		/* Convert irq_num to hexadecimal number corresponding to correct irq*/
		
		single_bit_mask = single_bit_mask << irq_num;

		/* Use OR so anything that is already disabled stays disabled */
		/* So a bit that's enabled (0) resolves to disabled (1) */
		master_mask = master_mask | single_bit_mask;

		outb( master_mask,  MASTER_8259_PORT_2);
		return;
	}


	/* Slave irq-- 8-15 */
	if(irq_num >= 8 && irq_num <= 15)
	{
		/* Minus 8 to recalculate offset starting from irq 8 in slave pic */
		offset = irq_num - 8;
		single_bit_mask = single_bit_mask << offset;

		/* Use OR so anything that is already disabled stays disabled */
		/* So a bit that's enabled (0) resolves to disabled (1) */
		slave_mask = slave_mask | single_bit_mask;

		outb( slave_mask,  SLAVE_8259_PORT_2);
		return;
	}


}

/* Send end-of-interrupt signal for the specified IRQ */
void
send_eoi(uint32_t irq_num)
{
	/* First check if irq_num is out of bounds */
	if(irq_num > 15 || irq_num < 0)
		return;

	/* 
	 * EOI byte is OR'd with the interrupt number and
	 * sent out to the PIC to declare the interrupt finished 
	 */


	/* If the IRQ is in master PIC, then send EOI only to master pic.
	 * However, if the IRQ is in slave PIC, have issue the command to
	 * both PICs */
	if(irq_num <= 7 && irq_num >= 0)
	{
		/* EOI signal to master pic regardless */
		uint32_t out_to_pic = irq_num | EOI;
		outb( out_to_pic, MASTER_8259_PORT);
	}
	
	/* Slave irq-- 8-15 */
	if(irq_num >= 8 && irq_num <= 15)
	{
		/* Minus 8 to recalculate offset starting from irq 8 in slave pic */
		uint32_t offset = irq_num - 8;
		outb( (EOI | offset), SLAVE_8259_PORT);
		/* SLAVE_IRQ is to account for irq master uses for slave */
		outb( (EOI + SLAVE_IRQ) , MASTER_8259_PORT);
	}

	

	return;
}














