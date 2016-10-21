#include "rtc.h"
#include "lib.h"
#include "i8259.h"

#define WIPE_MASK   0x80
#define DIV_AND_RS  0x26
#define TESTA       0xA6
#define TESTB       0x68
#define WIPE_INT    0x42
#define PER_INT_EN  0x40

void rtc_init(){
    unsigned char prv_a;
    unsigned char prv_b;

    //obtain the previous statuses of a and b
    outb(RTC_REG_A, RTC_PORT);
    prv_a = inb(RTC_CMOS);
    outb(RTC_REG_B, RTC_PORT);
    prv_b = inb(RTC_CMOS);

    //Enable RTC with divider freq 1.024 kHz (bits [3:0] = 0110),
    //22 stage divider (bits [6:4] = 010) initialized to 32.768 kHz,   
    outb(RTC_REG_A, RTC_PORT);
    outb((prv_a & WIPE_MASK)|DIV_AND_RS, RTC_CMOS);

    //Enable periodic interrupts from Status Reg B
    outb(RTC_REG_B, RTC_PORT);
    outb(prv_b | PER_INT_EN, RTC_CMOS);

    //open the irq line for the RTC
    enable_irq(RTC_IRQ);
}

/*  rtc_irq_handler
 *   temporary interrupt handler for RTC
 *   masks own IRQ line so as to not interrupt itself
 *   
 */
void rtc_irq_handler(){

    //Mask own irq line to prevent being interrupted by itself
    disable_irq(RTC_IRQ);
    //cli();
    //Obtain contents of CMOS register C (don't really care about contents)
    outb(INDEX_REG_C,RTC_PORT);
    inb(RTC_CMOS);

    // test_interrupts as a check
    //test_interrupts();

    //end of interrupt done
    send_eoi(RTC_IRQ);
    
    //re-enable irq line
    enable_irq(RTC_IRQ);
    sti();
}
