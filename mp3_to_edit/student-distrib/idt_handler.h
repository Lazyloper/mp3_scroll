#ifndef IDT_HANDLER_H
#define IDT_HANDLER_H


//the handler for all interrupt and exception
extern void DIVIDE_ERROR_handler();

extern void RESERVED_handler();

extern void NMI_INTERRUPT_handler();

extern void BREAKPOINT_handler();

extern void OVERFLOW_handler();

extern void BOUND_RANGE_EXCEEDED_handler();

extern void INVALID_OPCODE_handler();

extern void DEVICE_NOT_AVAILABLE_handler();

extern void DOUBLE_FAULT_handler();

extern void COPROCESSOR_SEGMENT_handler();

extern void INVALID_TSS_handler();

extern void SEGMENT_NOT_PRESENT_handler();

extern void STACK_SEGMENT_FAULT_handler();

extern void GENERAL_PROTECTION_handler();

extern void PAGE_FAULT_handler();

extern void INTEL_RESERVED_handler();

extern void FLOATING_POINT_ERROR_handler();

extern void ALIGNMENT_CHECK_handler();

extern void MACHINE_CHECK_handler();

extern void FLOATING_POINT_EXCEPTION_handler();

#endif
