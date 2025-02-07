#include "scheduler.h"
#include "i8259.h"
#include "system_call.h"
#include "keyboard.h"
#include "lib.h"
/* pit_init()
* INPUT: NONE
* OUTPUT: NONE
* RETURN VAL: NONE
*   Function that 1) sets it to square wave generation mode 2) sets the period to 40 ms and 3) enables its IRQ line
*/
void pit_init()
{
    //set the operation to square wave mode, lo/hi byte access mode
    outb(PIT_MODE_3, PIT_CMD_REG_PT);
    //set the period to 40 ms, low byte -> high byte order
    outb(FREQ_DIV_PIT & LO_8_MASK, PIT_CHNL_0);
    outb(FREQ_DIV_PIT >> HI_8_BITSHIFT, PIT_CHNL_0);

    //enable PIT IRQ Line
    enable_irq(PIT_IRQ);
}
/* pit_irq_sched_handler
* INPUT: NONE
* OUTPUT: NONE
* RETURN VALUE: NONE
* Function is "supposed to" implement a round robin scheduler. Supposed to. Currently does nothing.
*/
void pit_irq_sched_handler()
{
    //acknowledge that IRQ is being handled
    send_eoi(PIT_IRQ);
    cli();
    //printf("weeeee\n");
    sti();
    //disable interrupts while performing
    /*cli();
    pcb_t* cur_PCB;
    uint32_t next_proc_num;
    //int i;

    //calculate current process               
    asm volatile(
            "movl %%esp, %0   \n"
            : "=r" (cur_PCB)
            : //no inputs
            );

    //cur_PCB = cur_PCB & PCB_MASK;



    //process to be switched to / scheduled
    next_proc_num = (cur_PCB->proc_num + 1) % MAX_PROCESSES;

    //if next process to be switched/scheduled to is not active, find next active
    while( proc_id_flags[next_proc_num] == 0)
    {
        next_proc_num++;

        if(next_proc_num >= MAX_PROCESSES)
            next_proc_num %= MAX_PROCESSES;
    }

    //return if there is no other process available to switch to or schedule
    if(next_proc_num == cur_PCB->proc_num)
        return;

    //save EBP and ESP of current process before switching
    asm volatile(
              "movl   %%ebp, %0   \n"
              "movl   %%esp, %1   \n"
              : "=r"(cur_PCB->kbp), "=r"(cur_PCB->ksp) //outputs, ebp stored in cur_pcb->kbp
              : //no inputs
              );

    //switch to next active/running process
    cur_PCB = (pcb_t *)(PAGE_8MB-STACK_8KB*(next_proc_num + 1));

    
    //update the now current process that is running in file system_call.c
    //so program knows what process to execute other processes from.



    //update correct terminal id of next process
    set_cur_term_id(cur_PCB->active_term_number); 



    //update page table of next process
    mapUserImgPage(cur_PCB->proc_num);


    //tss to point to bottom of next process's kernel stack
    tss.esp0 = PAGE_8MB-STACK_8KB*(cur_PCB->proc_num)-4;



    sti();*/
}




















