#ifndef _HANDLERS_H_
#define _HANDLERS_H_
//Handlers for interrupts
extern void keyboard_handler();

extern void rtc_handler();

extern void main_syscall();

extern void pit_handler();

#endif
