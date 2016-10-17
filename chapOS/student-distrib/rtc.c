#include "rtc.h"
#include "lib.h"
#include "i8259.h"

void rtc_init(){
    unsigned char prv_a;
    unsigned char prv_b;


}

void rtc_handler(){
    cli();

    outb(INDEX_REG_C,RTC_PORT);
    inb(CMOS_PORT);

    test_interrupts();
    sendeoi(RTC_IRQ);
    
    sti();
}