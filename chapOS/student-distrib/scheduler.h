#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#define PIT_IRQ         0
#define PIT_CMD_REG_PT  0x43
#define PIT_CHNL_0      0x40
#define PIT_MODE_3      0x36

void pit_init();

#endif