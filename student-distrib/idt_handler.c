#include "idt_handler.h"
#include "lib.h"
#include "SystemCall.h"

//the handler for all interrupt and exception
// the order is the same as the intel
//for CP1, we just print all handler name for each interrupt and exception


// DIVIDE_ERROR_handler:
// description: handle the DIVIDE_ERROR exception
// input:None
// output:None
void DIVIDE_ERROR_handler() {
		printf("DIVIDE_ERROR_handler \n");
		while(1);
}

// RESERVED_handler():
// description: handle the DIVIDE_ERROR exception
// input:None
// output:None
void RESERVED_handler() {
		printf("RESERVED_handler \n");
		while(1);
}

// NMI_INTERRUPT_handler():
// description: handle the DIVIDE_ERROR exception
// input:None
// output:None
void NMI_INTERRUPT_handler() {
		printf("NMI_INTERRUPT_handler \n");
		while(1);
}

// BREAKPOINT_handler():
// description: handle the DIVIDE_ERROR exception
// input:None
// output:None
void BREAKPOINT_handler() {
		printf("BREAKPOINT_handler \n");
		while(1);
}

// OVERFLOW_handler:
// description: handle the DIVIDE_ERROR exception
// input:None
// output:None
void OVERFLOW_handler() {
		printf("OVERFLOW_handler \n");
		while(1);
}

// BOUND_RANGE_EXCEEDED_handler():
// description: handle the DIVIDE_ERROR exception
// input:None
// output:None
void BOUND_RANGE_EXCEEDED_handler() {
		printf("BOUND_RANGE_EXCEEDED_handler \n");
		while(1);
}

// INVALID_OPCODE_handler():
// description: handle the DIVIDE_ERROR exception
// input:None
// output:None
void INVALID_OPCODE_handler() {
		printf("INVALID_OPCODE_handler \n");
		while(1);
}

// DEVICE_NOT_AVAILABLE_handler():
// description: handle the DIVIDE_ERROR exception
// input:None
// output:None
void DEVICE_NOT_AVAILABLE_handler() {
		printf("DEVICE_NOT_AVAILABLE_handler \n");
		while(1);
}

// DOUBLE_FAULT_handler():
// description: handle the DIVIDE_ERROR exception
// input:None
// output:None
void DOUBLE_FAULT_handler() {
		printf("DOUBLE_FAULT_handler \n");
		while(1);
}

//  COPROCESSOR_SEGMENT_handler():
// description: handle the DIVIDE_ERROR exception
// input:None
// output:None
void COPROCESSOR_SEGMENT_handler() {
		printf("COPROCESSOR_SEGMENT_handler \n");
		while(1);
}

//  INVALID_TSS_handler(:
// description: handle the DIVIDE_ERROR exception
// input:None
// output:None
void INVALID_TSS_handler() {
		printf("INVALID_TSS_handler \n");
		while(1);
}

// SEGMENT_NOT_PRESENT_handler():
// description: handle the DIVIDE_ERROR exception
// input:None
// output:None
void SEGMENT_NOT_PRESENT_handler() {
		printf("SEGMENT_NOT_PRESENT_handler \n");
		while(1);
}

// STACK_SEGMENT_FAULT_handler():
// description: handle the STACK_SEGMENT_FAULT exception
// input:None
// output:None
void STACK_SEGMENT_FAULT_handler() {
		printf("STACK_SEGMENT_FAULT_handler \n");
		while(1);
}

// GENERAL_PROTECTION_handler():
// description: handle the DIVIDE_ERROR exception
// input:None
// output:None
void GENERAL_PROTECTION_handler() {
		printf("GENERAL_PROTECTION_handler \n");
		// halt(256);
		while(1);
}

// PAGE_FAULT_handler():
// description: handle the DIVIDE_ERROR exception
// input:None
// output:None
void PAGE_FAULT_handler() {
		printf("PAGE_FAULT_handler \n");
		halt(256);			// 256 because it halts by exception
		while(1);
}

// INTEL_RESERVED_handler():
// description: handle the DIVIDE_ERROR exception
// input:None
// output:None
void INTEL_RESERVED_handler() {
		printf("INVALID_TSS_handler \n");
		while(1);
}

// FLOATING_POINT_ERROR_handler():
// description: handle the DIVIDE_ERROR exception
// input:None
// output:None
void FLOATING_POINT_ERROR_handler() {
		printf("FLOATING_POINT_ERROR_handler \n");
		while(1);
}

// ALIGNMENT_CHECK_handler():
// description: handle the DIVIDE_ERROR exception
// input:None
// output:None
void ALIGNMENT_CHECK_handler() {
		printf("ALIGNMENT_CHECK_handler \n");
		while(1);
}

// MACHINE_CHECK_handler():
// description: handle the DIVIDE_ERROR exception
// input:None
// output:None
void MACHINE_CHECK_handler() {
		printf("MACHINE_CHECK_handler \n");
		while(1);
}

// FLOATING_POINT_EXCEPTION_handler():
// description: handle the DIVIDE_ERROR exception
// input:None
// output:None
void FLOATING_POINT_EXCEPTION_handler() {
		printf("FLOATING_POINT_ERROR_handler \n");
		while(1);
}
