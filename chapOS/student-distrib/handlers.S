# interrupt handlers written in assembly because iret has to come before C function's leave and ret
# thus making it pointless.
# iret has to be used because these are interrupts and only iret returns to the original context

.global rtc_handler
.global keyboard_handler

rtc_handler:
    pushl   %eax
    pushl   %ecx
    pushl   %edx

    call rtc_irq_handler

    popl    %edx
    popl    %ecx
    popl    %eax

    iret

keyboard_handler:
    pushl   %eax
    pushl   %ecx
    pushl   %edx

    call keyboard_int_handler

    popl    %edx
    popl    %ecx
    popl    %eax    

    iret
