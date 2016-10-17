# interrupt handlers must be in x86, because is an interrupt
#  
.global rtc_handler
.global keyboard_handler

rtc_handler:
    pushl   %eax
    pushl   %ecx
    pushl   %edx

    call keyboard_handler

    popl    %edx
    popl    %ecx
    popl    %eax

    iret

keyboard_handler:
    pushl   %eax
    pushl   %ecx
    pushl   %edx

    call rtc_handler

    popl    %edx
    popl    %ecx
    popl    %eax    

    iret