#include "rtc.h"
#include "lib.h"
#include "i8259.h"

#define WIPE_MASK   0x80
#define DIV_AND_RS  0x26
#define WIPE_INT    0x8F
#define PER_INT_EN  0x40

void rtc_init(){
    unsigned char prv_a;
    unsigned char prv_b;

    //obtain the previous statuses of a and b
    outb(INDEX_REG_A, RTC_PORT);
    prv_a = inb(CMOS_PORT);
    outb(INDEX_REG_B, RTC_PORT);
    prv_b = inb(CMOS_PORT);

    outb(INDEX_REG_A, RTC_PORT);
    outb((WIPE_MASK & prv_a) | DIV_AND_RS, CMOS_PORT );
    outb(INDEX_REG_B, RTC_PORT);
    outb((WIPE_INT & prv_b) | PER_INT_EN, CMOS_PORT );

    enable_irq(RTC_IRQ);
}

void rtc_handler(){
    disable_irq(RTC_IRQ);

    outb(INDEX_REG_C,RTC_PORT);
    inb(CMOS_PORT);

    test_interrupts();
    sendeoi(RTC_IRQ);
    
    enable_irq(RTC_IRQ);
}