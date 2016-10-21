# interrupt handlers written in assembly because iret has to come before C function's leave and ret
# thus making it pointless.
# iret has to be used because these are interrupts and only iret returns to the original context

.global rtc_handler
.global keyboard_handler

rtc_handler:
    pushl   %eax
    pushl   %ecx
    pushl   %edx
    pushl   %ebx
    pushl   %esi
    pushl   %edi

    call rtc_irq_handler

    popl    %edi
    popl    %esi
    popl    %ebx
    popl    %edx
    popl    %ecx
    popl    %eax

    iret

keyboard_handler:
    pushl   %eax
    pushl   %ecx
    pushl   %edx
    pushl   %ebx
    pushl   %esi
    pushl   %edi

    call keyboard_int_handler

    popl    %edi
    popl    %esi
    popl    %ebx
    popl    %edx
    popl    %ecx
    popl    %eax    

    iret
