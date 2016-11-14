.globl main_syscall

# The main system call function that will call correspond to the EAX
# EAX: The number of system call to call
# EBX: 1st arg
# ECX: 2nd arg
# EDX: 3rd arg
main_syscall:
    pushfl
    subl    $4, %esp
    pushal
    leal    32(%esp), %esi
    cmpl    $10, %eax
    jg      ret_err
    cmpl    $1, %eax
    jl      ret_err
    call    syscall_jmp(%eax)
    movl    %eax, (%esi)
    popal
    movl    (%esp), %eax
    addl    $4, %esp
    popfl
    iret
ret_err:
    popal
    leal    -1, %eax       # return -1 for error
    addl    $4, %esp
    popfl
    iret

    
# The jump table
syscall_jmp:
    .long 0, _halt, _execute, _read, _write, _open, _close, _getargs, _vidmap

# Below are the functions for passing arguments for each sys call functions
# Passing arg for halt
_halt:
    pushl   %ebx
    call    halt
    popl    %ebx
    ret

# Passing arg for execute
_execute:
    pushl   %ebx
    call    execute
    popl    %ebx
    ret

# Passing args for read
_read:
    pushl   %edx
    pushl   %ecx
    pushl   %ebx
    call    read
    popl    %ebx
    popl    %ecx
    popl    %edx
    ret

# Passing args for write
_write:
    pushl   %edx
    pushl   %ecx
    pushl   %ebx
    call    write
    popl    %ebx
    popl    %ecx
    popl    %edx
    ret

# Passing arg for open
_open:
    pushl   %ebx
    call    open
    popl    %ebx
    ret

# Passing arg for close
_close:
    pushl   %ebx
    call    close
    popl    %ebx
    ret

# Passing args for getargs
_getargs:
    pushl   %ecx
    pushl   %ebx
    call    getargs
    popl    %ebx
    popl    %ecx
    ret

# Passing arg for vidmap
_vidmap:
    pushl   %ebx
    call    vidmap
    popl    %ebx
    ret
