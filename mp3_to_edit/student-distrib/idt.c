#include "idt.h"
#include "idt_handler.h"
#include "isr.h"
#include "SystemCall.h"

// idt_init():
// description: this function is used to initial the interrrupt description table
// input: None
// output:none:
// side effect: idt will be filled with their handler
void idt_init(){
  int i; // for index

//for all interrupt,set the following value:
// seg_selector : KERNEL_CS
// size : 1
// dpl : 0
// present: 1
// reserved : 01110
  for (i=0; i < 32; i++){
    idt[i].seg_selector = KERNEL_CS;
    idt[i].reserved4 = 0;
    idt[i].size = 1;

    idt[i].reserved3 = 1;
    idt[i].reserved2 = 1;
    idt[i].reserved1 = 1;
    idt[i].reserved0 = 0;
    idt[i].dpl = 0;
    idt[i].present = 1;
  }
//for all interrupt,set the following value:
// seg_selector : KERNEL_CS
// size : 1
// dpl : 0
// present: 1
// reserved : 01100
  for (i=32; i < NUM_VEC; i++){
    idt[i].seg_selector = KERNEL_CS;
    idt[i].reserved4 = 0;
    idt[i].size = 1;

    idt[i].reserved3 = 0;
    idt[i].reserved2 = 1;
    idt[i].reserved1 = 1;
    idt[i].reserved0 = 0;
    idt[i].dpl = 0;
    idt[i].present = 1;
  }

  idt[128].dpl = 3;

  //load all idt into the table using SET_IDT_ENTRY()
  SET_IDT_ENTRY(idt[0], intr_00);
  SET_IDT_ENTRY(idt[1], intr_01);
  SET_IDT_ENTRY(idt[2], intr_02);
  SET_IDT_ENTRY(idt[3], intr_03);
  SET_IDT_ENTRY(idt[4], intr_04);
  SET_IDT_ENTRY(idt[5], intr_05);
  SET_IDT_ENTRY(idt[6], intr_06);
  SET_IDT_ENTRY(idt[7], intr_07);
  SET_IDT_ENTRY(idt[8], intr_08);
  SET_IDT_ENTRY(idt[9], intr_09);
  SET_IDT_ENTRY(idt[10], intr_0A);
  SET_IDT_ENTRY(idt[11], intr_0B);
  SET_IDT_ENTRY(idt[12], intr_0C);
  SET_IDT_ENTRY(idt[13], intr_0D);
  SET_IDT_ENTRY(idt[14], intr_0E);
  SET_IDT_ENTRY(idt[15], intr_0F);
  SET_IDT_ENTRY(idt[16], intr_10);
  SET_IDT_ENTRY(idt[17], intr_11);
  SET_IDT_ENTRY(idt[18], intr_12);
  SET_IDT_ENTRY(idt[19], intr_13);
  SET_IDT_ENTRY(idt[20], intr_01);
  SET_IDT_ENTRY(idt[21], intr_01);    //all these unused ENTRY set to reserved
  SET_IDT_ENTRY(idt[22], intr_01);
  SET_IDT_ENTRY(idt[23], intr_01);
  SET_IDT_ENTRY(idt[24], intr_01);
  SET_IDT_ENTRY(idt[25], intr_01);
  SET_IDT_ENTRY(idt[26], intr_01);
  SET_IDT_ENTRY(idt[27], intr_01);
  SET_IDT_ENTRY(idt[28], intr_01);
  SET_IDT_ENTRY(idt[29], intr_01);
  SET_IDT_ENTRY(idt[30], intr_01);
  SET_IDT_ENTRY(idt[31], intr_01);
  SET_IDT_ENTRY(idt[32], intr_20);
  SET_IDT_ENTRY(idt[33], intr_21);    //0x20 for keyboard interrupt
  SET_IDT_ENTRY(idt[40], intr_28);    // 0x28 for RTC interrupt
  SET_IDT_ENTRY(idt[128], SYSTEM_CALL_handler);    // 0x80 for system call
}
