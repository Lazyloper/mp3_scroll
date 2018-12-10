
.data
      SYS_HALT  = 1
      SYS_EXECUTE = 2
      SYS_READ    = 3
      SYS_WRITE   = 4
      SYS_OPEN    = 5
      SYS_CLOSE   = 6
      SYS_GETARGS = 7
      SYS_VIDMAP  = 8
      SYS_SET_HANDLER  = 9
      SYS_SIGRETURN  = 10
      SYSCALL_INTERRUPT = 0x80

      FIRST_ARGUMENTS = 8
      SECOND_ARGUMENTS = 12
      THIRD_ARGUMENTS = 16


.text
.globl SYSTEM_CALL_handler
.globl halt,execute,read,write,open,close,getargs,vidmap,set_handler,sigreturn
.align 4
return_val:
  .long 0x00
SYSTEM_CALL_handler:

  cmpl $0,%eax             #0 is the lower bound of syscall number
  jle bad_return
  cmpl $10, %eax          #10 is the upper bound of the syscall number
  jg bad_return

  cli
  pushal
  pushfl
  pushl %eax
  pushl %edx
  pushl %ecx
  pushl %ebx
  sti
  call *syscall_jump_table-4(, %eax, 4)       # 4 bytes for each long type
  addl $16, %esp                              # 4 bytes for one register and we have 4 regisiters
  movl %eax, return_val
  popfl
  popal
  movl return_val, %eax
  jmp return

bad_return:
  movl $-1, %eax

return:
  iret

syscall_jump_table:
  .long  system_halt
  .long  system_execute
  .long  system_read
  .long  system_write
  .long  system_open
  .long  system_close
  .long  system_getargs
  .long  system_vidmap
  .long  system_set_handler
  .long  system_sigreturn

halt:
  PUSHL	%EBX
  MOVL	$SYS_HALT,%EAX						#1 because halt system call number
  MOVL	FIRST_ARGUMENTS(%ESP),%EBX				# save all register
  MOVL	SECOND_ARGUMENTS(%ESP),%ECX
  MOVL	THIRD_ARGUMENTS(%ESP),%EDX
  INT	$SYSCALL_INTERRUPT
  POPL	%EBX
  RET

execute:
  PUSHL	%EBX
  MOVL	$SYS_EXECUTE,%EAX						#2 because halt system call number
  MOVL	FIRST_ARGUMENTS(%ESP),%EBX				# save all register
  MOVL	SECOND_ARGUMENTS(%ESP),%ECX
  MOVL	THIRD_ARGUMENTS(%ESP),%EDX
  INT	$SYSCALL_INTERRUPT
  POPL	%EBX
  RET

read:
  PUSHL	%EBX
  MOVL	$SYS_READ,%EAX						#3 because halt system call number
  MOVL	FIRST_ARGUMENTS(%ESP),%EBX				# save all register
  MOVL	SECOND_ARGUMENTS(%ESP),%ECX
  MOVL	THIRD_ARGUMENTS(%ESP),%EDX
  INT	$SYSCALL_INTERRUPT
  POPL	%EBX
  RET

write:
  PUSHL	%EBX
  MOVL	$SYS_WRITE,%EAX						#4 because halt system call number
  MOVL	FIRST_ARGUMENTS(%ESP),%EBX					# save all register
  MOVL	SECOND_ARGUMENTS(%ESP),%ECX
  MOVL	THIRD_ARGUMENTS(%ESP),%EDX
  INT	$SYSCALL_INTERRUPT
  POPL	%EBX
  RET

open:
  PUSHL	%EBX
  MOVL	$SYS_OPEN,%EAX						#5 because halt system call number
  MOVL	FIRST_ARGUMENTS(%ESP),%EBX					# save all register
  MOVL	SECOND_ARGUMENTS(%ESP),%ECX
  MOVL	THIRD_ARGUMENTS(%ESP),%EDX
  INT	$SYSCALL_INTERRUPT
  POPL	%EBX
  RET

close:
  PUSHL	%EBX
  MOVL	$SYS_CLOSE,%EAX						#6 because halt system call number
  MOVL	FIRST_ARGUMENTS(%ESP),%EBX					# save all register
  MOVL	SECOND_ARGUMENTS(%ESP),%ECX
  MOVL	THIRD_ARGUMENTS(%ESP),%EDX
  INT	$SYSCALL_INTERRUPT
  POPL	%EBX
  RET

getargs:
  PUSHL	%EBX
  MOVL	$SYS_GETARGS,%EAX						#7 because halt system call number
  MOVL	FIRST_ARGUMENTS(%ESP),%EBX					# save all register
  MOVL	SECOND_ARGUMENTS(%ESP),%ECX
  MOVL	THIRD_ARGUMENTS(%ESP),%EDX
  INT	$SYSCALL_INTERRUPT
  POPL	%EBX
  RET

vidmap:
  PUSHL	%EBX
  MOVL	$SYS_VIDMAP,%EAX						#8 because halt system call number
  MOVL	FIRST_ARGUMENTS(%ESP),%EBX					# save all register
  MOVL	SECOND_ARGUMENTS(%ESP),%ECX
  MOVL	THIRD_ARGUMENTS(%ESP),%EDX
  INT	$SYSCALL_INTERRUPT
  POPL	%EBX
  RET

set_handler:
  PUSHL	%EBX
  MOVL	$SYS_SET_HANDLER,%EAX						#9 because halt system call number
  MOVL	FIRST_ARGUMENTS(%ESP),%EBX
  MOVL	SECOND_ARGUMENTS(%ESP),%ECX					# save all register
  MOVL	THIRD_ARGUMENTS(%ESP),%EDX
  INT	$SYSCALL_INTERRUPT
  POPL	%EBX
  RET

sigreturn:
  PUSHL	%EBX
  MOVL	$SYS_SIGRETURN,%EAX					#10 because halt system call number
  MOVL	FIRST_ARGUMENTS(%ESP),%EBX
  MOVL	SECOND_ARGUMENTS(%ESP),%ECX					# save all register
  MOVL	THIRD_ARGUMENTS(%ESP),%EDX
  INT	$SYSCALL_INTERRUPT
  POPL	%EBX
  RET
