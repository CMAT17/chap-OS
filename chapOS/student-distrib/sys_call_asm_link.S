.globl main_syscall, _execute_iret, RET_FROM_IRET

# The main system call function that will call correspond to the EAX
# EAX: The number of system call to call
# EBX: 1st arg
# ECX: 2nd arg
# EDX: 3rd arg
main_syscall:
    cli
    pushfl
    #subl    $4, %esp
    pushal
    leal    32(%esp), %esi
    cmpl    $10, %eax
    jg      ret_err
    cmpl    $1, %eax
    jl      ret_err
    call    *syscall_jmp(,%eax,4)
    #movl    %eax, (%esi)
    movl    %eax, (main_syscall_retval)
    popal
    #movl    (%esp), %eax
    #addl    $4, %esp
    movl    (main_syscall_retval), %eax
    popfl
    sti
    iret
ret_err:
    popal
    leal    -1, %eax       # return -1 for error
    #addl    $4, %esp
    popfl
    sti
    iret

.align 4
main_syscall_retval:
  .long 0
    
# The jump table
syscall_jmp:
    .long 0,_halt, _execute, _read, _write, _open, _close, _getargs, _vidmap

# Below are the functions for passing arguments for each sys call functions
# Passing arg for halt
_halt:
    pushl   %ebx
    sti
    call    halt
    cli
    popl    %ebx
    ret

# Passing arg for execute
_execute:
    pushl   %ebx
    sti
    call    execute
    cli
    popl    %ebx
    ret

# Passing args for read
_read:
    pushl   %edx
    pushl   %ecx
    pushl   %ebx
    sti
    call    read
    cli
    popl    %ebx
    popl    %ecx
    popl    %edx
    ret

# Passing args for write
_write:
    pushl   %edx
    pushl   %ecx
    pushl   %ebx
    sti
    call    write
    cli
    popl    %ebx
    popl    %ecx
    popl    %edx
    ret

# Passing arg for open
_open:
    pushl   %ebx
    sti
    call    open
    cli
    popl    %ebx
    ret

# Passing arg for close
_close:
    pushl   %ebx
    sti
    call    close
    cli
    popl    %ebx
    ret

# Passing args for getargs
_getargs:
    pushl   %ecx
    pushl   %ebx
    sti
    call    getargs
    cli
    popl    %ebx
    popl    %ecx
    ret

# Passing arg for vidmap
_vidmap:
    pushl   %ebx
    sti
    call    vidmap
    cli
    popl    %ebx
    ret

_execute_iret:
  cli
  movw   $0x2B,%ax
  movw   %ax, %ds
  pushl  $0x2B
  movl   $0x083FFFFC, %eax
  pushl  %eax
  pushfl
  popl   %eax
  orl    $0x4200, %eax
  pushl  %eax
  pushl  $0x23
  #movl   %0, %ebx
  pushl  %ebx
  iret
RET_FROM_IRET:
  leave
  sti
  ret



