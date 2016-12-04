#include "scheduler.h"
#include "i8259.h"
#include "syscalls.h"
#include "lib.h"

#define FREQ_DIV_PIT    2984
#define LO_8_MASK       0x00FF
#define HI_8_BITSHIFT   8

void pit_init()
{
    //set the operation to square wave mode, lo/hi byte access mode
    outb(PIT_MODE_3, PIT_CMD_REG_PT);
    //set the frequency to 40 Hz, low byte -> high byte order
    outb(FREQ_DIV_PIT & LO_8_MASK, PIT_CHNL_0);
    outb(FREQ_DIV_PIT >> HI_8_BITSHIFT, PIT_CHNL_0);

    //enable PIT IRQ Line
    enable_irq(PIT_IRQ);
}

void pit_irq_sched_handler()
{
    //acknowledge that IRQ is being handled
    send_eoi(PIT_IRQ);
    //disable interrupts while performing
    cli();

    

    sti();
}