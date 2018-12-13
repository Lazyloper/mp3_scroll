/* filename: isr_wrapper.s */


.text
.globl   intr_00,intr_01,intr_02,intr_03,intr_04,intr_05,intr_06,intr_07,intr_08,intr_09,
.globl   intr_0A,intr_0B,intr_0C,intr_0D,intr_0E,intr_0F,intr_10,intr_11,intr_12,intr_13,intr_20,intr_21,intr_28,intr_80
.globl   test_DIVIDE_ERROR,test_OVERFLOW,test_STACK_SEGMENT_FAULT,SYSTEM_CALL_handler
.align   4


#  for each interrupt, push its index and we can find its handle function by its index
intr_00:
  pushl $0x00                 # 00 beacuse it is an exception and it is in index 0
  jmp common_handler


intr_01:
  pushl $0x01                 # 01 beacuse it is an exception and it is in index 1
  jmp common_handler


intr_02:
  pushl $0x02                 # 02 beacuse it is an exception and it is in index 2
  jmp common_handler


intr_03:
  pushl $0x03                 # 03 beacuse it is an exception and it is in index 3
  jmp common_handler


intr_04:
  pushl $0x04                 # 04 beacuse it is an exception and it is in index 4
  jmp common_handler


intr_05:
  pushl $0x05                 # 05 beacuse it is an exception and it is in index 5
  jmp common_handler


intr_06:
  pushl $0x06                 # 06 beacuse it is an exception and it is in index 6
  jmp common_handler


intr_07:
  pushl $0x07                 # 07 beacuse it is an exception and it is in index 7
  jmp common_handler


intr_08:
  pushl $0x08                 # 08 beacuse it is an exception and it is in index 8
  jmp common_handler


intr_09:
  pushl $0x09                 # 09 beacuse it is an exception and it is in index 9
  jmp common_handler


intr_0A:
  pushl $0x0A                 # 0A beacuse it is an exception and it is in index 10
  jmp common_handler


intr_0B:
  pushl $0x0B                 # 0B beacuse it is an exception and it is in index 11
  jmp common_handler

intr_0C:
  pushl $0x0C                 # 0C beacuse it is an exception and it is in index 12
  jmp common_handler


intr_0D:
  pushl $0x0D                 # 0D beacuse it is an exception and it is in index 13
  jmp common_handler


intr_0E:
  pushl $0x0E                 # 0E beacuse it is an exception and it is in index 14
  jmp common_handler


intr_0F:
  pushl $0x0F                 # 0F beacuse it is an exception and it is in index 15
  jmp common_handler


intr_10:
  pushl $0x10                 # 10 beacuse it is an exception and it is in index 16
  jmp common_handler


intr_11:
  pushl $0x11                 # 11 beacuse it is an exception and it is in index 17
  jmp common_handler


intr_12:
  pushl $0x12                 # 12 beacuse it is an exception and it is in index 18
  jmp common_handler


intr_13:                      # 13 beacuse it is an exception and it is in index 19
  pushl $0x13
  jmp common_handler

intr_20:
  pushl $-33                # -33 beacuse it is a interrupt and it is in index 32
  jmp common_handler

intr_21:
  pushl $-34                # -34 beacuse it is a interrupt and it is in index 33
  jmp common_handler


intr_28:
  pushl $-41                # -41 becasue it is a interrupt and it is in index 40
  jmp common_handler


intr_80:
                            # -41 becasue it is a interrupt and it is in index 40
  jmp SYSTEM_CALL_handler


# common_handler:
# description: determine whether it is a interrupt or an exception, call its handle function
# input:none
# output:None
# side effect: find its handler funciton


  common_handler:
    # get pushed number from stack
    pusha
    movl 32(%esp), %eax         #32 becasuse pusha has push 8 registers, which register occupy 4 bytes
    cmpl $0, %eax
    jl INTERRUPT
    cli
    jmp handle
INTERRUPT:
    notl %eax
handle:
    call *interrupt_jump_table(, %eax, 4)       # 4 bytes for each long type
    popa
    addl $4, %esp                 #add 4 to pop the index
    iret

#test_DIVIDE_ERROR:
#desciption: test the exception DIVIDE_ERROR
# input:none
# output:None
# side effect: the exception occurs
   test_DIVIDE_ERROR:
      int $0x00
      ret
#test_OVERFLOWR:
#desciption: test the exception OVERFLOW
# input:none
# output:None
# side effect: the exception occurs
   test_OVERFLOW:
      int $0x04
      ret
#test_STACK_SEGMENT_FAULT:
#desciption: test the exception STACK_SEGMENT_FAULT
# input:none
# output:None
# side effect: the exception occurs
   test_STACK_SEGMENT_FAULT:
      int $0x0C
      ret





# the idt for us, the top 20 is the same as Intel
  interrupt_jump_table:
  .long     DIVIDE_ERROR_handler
  .long     RESERVED_handler
  .long     NMI_INTERRUPT_handler
  .long     BREAKPOINT_handler
  .long     OVERFLOW_handler
  .long     BOUND_RANGE_EXCEEDED_handler
  .long     INVALID_OPCODE_handler
  .long     DEVICE_NOT_AVAILABLE_handler
  .long     DOUBLE_FAULT_handler
  .long     COPROCESSOR_SEGMENT_handler
  .long     INVALID_TSS_handler
  .long     SEGMENT_NOT_PRESENT_handler
  .long     STACK_SEGMENT_FAULT_handler
  .long     GENERAL_PROTECTION_handler
  .long     PAGE_FAULT_handler
  .long     INTEL_RESERVED_handler
  .long     FLOATING_POINT_ERROR_handler
  .long     ALIGNMENT_CHECK_handler
  .long     MACHINE_CHECK_handler
  .long     FLOATING_POINT_EXCEPTION_handler
  .rept 12
  .long     RESERVED_handler
  .endr
  .long     scheduling                    #IRQ0 , no matter, just give the value 1234
  .long     keyboard_interrupt_handler
  .rept 6
  .long 0
  .endr
  .long     rtc_interrupt_handler
  .rept 87
  .long     RESERVED_handler
  .endr
  .long     SYSTEM_CALL_handler      #system call is index 0x80
