#include "rtc.h"
#include "lib.h"
#include "i8259.h"

#define WIPE_MASK   0x80
#define DIV_AND_RS  0x2F
#define WIPE_INT    0x8F
#define PER_INT_EN  0x40

void rtc_init(){
    unsigned char prv_a;
    unsigned char prv_b;

    //obtain the previous statuses of a and b
    outb(INDEX_REG_A, RTC_PORT);
    prv_a = inb(RTC_CMOS);
    outb(INDEX_REG_B, RTC_PORT);
    prv_b = inb(RTC_CMOS);

    outb(INDEX_REG_A, RTC_PORT);
    outb((WIPE_MASK & prv_a) | DIV_AND_RS, RTC_CMOS );
    outb(INDEX_REG_B, RTC_PORT);
    outb((WIPE_INT & prv_b) | PER_INT_EN, RTC_CMOS );

    enable_irq(RTC_IRQ);
}

void rtc_irq_handler(){
    disable_irq(RTC_IRQ);

    outb(INDEX_REG_C,RTC_PORT);
    inb(RTC_CMOS);

    test_interrupts();
    send_eoi(RTC_IRQ);
    
    enable_irq(RTC_IRQ);
}
