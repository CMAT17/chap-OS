#ifndef _RTC_H_
#define _RTC_H_

#include "types.h"
//port info
#define RTC_PORT    0x70
#define RTC_CMOS    0x71

//indexes for each register
#define INDEX_REG_A     0x8A
#define INDEX_REG_B     0x8B
#define INDEX_REG_C     0x8C

//RTC Registers
#define RTC_SECONDS     0x00
#define RTC_SEC_ALARM   0x01
#define RTC_MINUTES     0x02
#define RTC_MIN_ALARM   0x03
#define RTC_HOURS       0x04
#define RTC_HRS_ALARM   0x05
#define RTC_REG_A       0x0A
#define RTC_REG_B       0x0B
#define RTC_REG_C       0x0C

//IRQ port number
#define RTC_IRQ         0x08

void rtc_init();
void rtc_irq_handler();

//-------------------------Start Sandwich Part---------------------------------

int32_t rtc_read();
int32_t rtc_set_rate(const int32_t int_rate);
int32_t rtc_open(const uint8_t* filename);
int32_t rtc_close(int32_t fd);

//-------------------------End Sandwich Part-----------------------------------

#endif
