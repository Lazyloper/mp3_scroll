/* filename: isr_wrapper.s */


#ifndef ISR_H
#define ISR_H


//extern isr_wrapper();
extern void intr_00();
extern void intr_01();
extern void intr_02();
extern void intr_03();
extern void intr_04();
extern void intr_05();
extern void intr_06();
extern void intr_07();
extern void intr_08();
extern void intr_09();
extern void intr_0A();
extern void intr_0B();
extern void intr_0C();
extern void intr_0D();
extern void intr_0E();
extern void intr_0F();
extern void intr_10();
extern void intr_11();
extern void intr_12();
extern void intr_13();
extern void intr_20();
extern void intr_21();
extern void intr_28();
extern void intr_80();
extern void test_DIVIDE_ERROR();
extern void test_OVERFLOW();
extern void test_STACK_SEGMENT_FAULT();

#endif
