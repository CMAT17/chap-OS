#include "x86_desc.h"
#include "idt.h"
#include "handler.h"
#include "rtc.h"
#include "keyboard.h"
#include "lib.h"

#define MASTER_PIC        0x20
#define EXCEPTION_NUM     32
#define KEYBOARD          0x21
#define RTC               0x28
#define SYSCALL_ENTRY     0x80

#define EXCEPT_FN(exception_name, msg)  \
static void exception_name()            \
{                                       \
    printf("%s\n", msg);                \
    while(1);                           \
}                                       \

static void generic_handler();

EXCEPT_FN(exception_DE,"Divide by Zero Error");
EXCEPT_FN(exception_DB,"Debug");
EXCEPT_FN(exception_NMI, "Non-maskable Interrupt");
EXCEPT_FN(exception_BP, "Breakpoint");
EXCEPT_FN(exception_OF, "Overflow");
EXCEPT_FN(exception_BR, "Beyond Range Exceeded");
EXCEPT_FN(exception_UD, "Invalid Opcode");
EXCEPT_FN(exception_NM, "Device Not Available");
EXCEPT_FN(exception_DF, "Double Fault");
EXCEPT_FN(exception_CO, "Coprocessor Segment Overrun");
EXCEPT_FN(exception_TS, "Invalid TSS");
EXCEPT_FN(exception_NP, "segment Not Present");
EXCEPT_FN(exception_SS, "Stack Segment Fault");
EXCEPT_FN(exception_GP, "General Protection Fault");
//EXCEPT_FN(exception_PF, "Page Fault");
EXCEPT_FN(exception_MF, "x87 Floating Point Exception");
EXCEPT_FN(exception_AC, "Alignment Check");
EXCEPT_FN(exception_MC, "Machine Check");
EXCEPT_FN(exception_XM, "SIMD Floating-Point Exception");
EXCEPT_FN(exception_VE, "Virtualization Exception");
EXCEPT_FN(exception_SX, "Security Exception");
//EXCEPT_FN(exception_TF, "Triple Fault");

static void exception_PF(){
    uint32_t cr2;
    //obtain linear address 
    asm("movl %%cr2, %0;"
        : "=r" (cr2)
        :
        : "cc" 
        );
    printf("Page Fault at %d", cr2);
    while(1);
}

//Generic handler
//placeholder handler for syscalls and interrupts
static void generic_handler(){
    printf("Interrupt/syscall unkown. pls halp \n");
}

void init_idt()
{
    int i; 

    idt_desc_t interrupt;
    idt_desc_t syscall;
    idt_desc_t exception;

    lidt(idt_desc_ptr);

    // least significant 8 bits are all 0
    interrupt.reserved4 = 0x0;
    exception.reserved4 = 0x0;
    syscall.reserved4 = 0x0;

    // Next LSB determines whether its a trap gate or an interrupt gate
    interrupt.reserved3 = 0x0;
    exception.reserved3 = 0x1;
    syscall.reserved3 = 0x1;

    // next few reserved's are hard coded for trap and interrupt gates
    interrupt.reserved2 = 0x1;
    exception.reserved2 = 0x1;
    syscall.reserved2 = 0x1;

    interrupt.reserved1 = 0x1;
    exception.reserved1 = 0x1;
    syscall.reserved1 = 0x1;

    interrupt.reserved0 = 0x0;
    exception.reserved0 = 0x0;
    syscall.reserved0 = 0x0;

    //must be set to zero for interrupt gate
    interrupt.size = 0x1;
    exception.size = 0x1;
    syscall.size = 0x1;

    // Set DP Level
    interrupt.dpl = 0;
    exception.dpl = 0;
    syscall.dpl = 3;

    // Set Present bit
    interrupt.present = 0x1;
    exception.present = 0x1;
    syscall.present = 0x1;
    
    // Seg Selector
    interrupt.seg_selector = exception.seg_selector = syscall.seg_selector = KERNEL_CS;

    // set the Offsets for each exception
    

    for(i = 0; i<NUM_VEC; i++)
    {
        if(i<EXCEPTION_NUM){
            idt[i] = exception;
            /*idt[i].reserved4 = 0x0;
            idt[i].reserved3 = 0x1;
            idt[i].reserved2 = 0x1;
            idt[i].reserved1 = 0x1;
            idt[i].size = 0x1;
            idt[i].reserved0 = 0x0;
            idt[i].dpl = 0;
            idt[i].present = 0x1;*/
        }
        else{

            idt[i] = interrupt;
            /*idt[i].reserved4 = 0x0;
            idt[i].reserved3 = 0x0;
            idt[i].reserved2 = 0x1;
            idt[i].reserved1 = 0x1;
            idt[i].size = 0x1;
            idt[i].reserved0 = 0x0;
            idt[i].dpl = 0;
            idt[i].present = 0x1;
    */
            if(i == SYSCALL_ENTRY)
            {
                idt[i] = syscall;
                /*idt[i].reserved4 = 0x0;
                idt[i].reserved3 = 0x1;
                idt[i].reserved2 = 0x1;
                idt[i].reserved1 = 0x1;
                idt[i].size = 0x1;
                idt[i].reserved0 = 0x0;
                idt[i].dpl = 3;
                idt[i].present = 0x1; */
            }
        }
    }

    //Sequentially set all the offsets for each interrupt
    SET_IDT_ENTRY(idt[KEYBOARD-1], generic_handler);
    //Set the keyboard handler offsets
    SET_IDT_ENTRY(idt[KEYBOARD], keyboard_handler);
    
    for(i = KEYBOARD+1; i <RTC; i++){
        SET_IDT_ENTRY(idt[i], generic_handler);
    }

    //Enable RTC Interrupts
    SET_IDT_ENTRY(idt[RTC], rtc_handler);

    for(i = RTC+1; i<NUM_VEC; i++){
        SET_IDT_ENTRY(idt[i],generic_handler);
    }
    

    //Fill in Segment data for exceptions
    SET_IDT_ENTRY(idt[0], exception_DE);
    SET_IDT_ENTRY(idt[1], exception_DB);
    SET_IDT_ENTRY(idt[2], exception_NMI);
    SET_IDT_ENTRY(idt[3], exception_BP);
    SET_IDT_ENTRY(idt[4], exception_OF);
    SET_IDT_ENTRY(idt[5], exception_BR);
    SET_IDT_ENTRY(idt[6], exception_UD);
    SET_IDT_ENTRY(idt[7], exception_NM);
    SET_IDT_ENTRY(idt[8], exception_DF);
    SET_IDT_ENTRY(idt[9], exception_CO);
    SET_IDT_ENTRY(idt[10], exception_TS);
    SET_IDT_ENTRY(idt[11], exception_NP);
    SET_IDT_ENTRY(idt[12], exception_SS);
    SET_IDT_ENTRY(idt[13], exception_GP);
    SET_IDT_ENTRY(idt[14], exception_PF);
    SET_IDT_ENTRY(idt[16], exception_MF);
    SET_IDT_ENTRY(idt[17], exception_AC);
    SET_IDT_ENTRY(idt[18], exception_MC);
    SET_IDT_ENTRY(idt[19], exception_XM);
    SET_IDT_ENTRY(idt[20], exception_VE);
    SET_IDT_ENTRY(idt[30], exception_SX);

    

    
}
