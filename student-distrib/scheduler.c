#include "scheduler.h"
#include "systemcall_function.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "paging.h"
#include "terminal.h"


#define PROGRAM_VIRTUAL_START 0x08000000

/* Segment selector values */
#define KERNEL_DS   0x0018      //00011000
#define TERMINAL_NUMBER 3

/* record the process numbers on each terminal */
uint32_t scheduled_processes[TERMINAL_NUMBER] = {0, 0, 0};

int32_t current_running_idx = 0;

/*   scheduling
 *   Inputs: none
 *   Return Value: none
 *   Function: schedule between the 3 terminals. Invoked on PIT interrupts.
 *   Switch stacks, served as PIT interrupt handler
 */
int32_t scheduling() {
    cli();

    pcb_t *next_pcb, *cur_pcb;
    uint32_t next_running_idx;
    int32_t cur_kesp, cur_kebp, next_kesp, next_kebp;

    /* send eoi as it's a interrupt handler */
    send_eoi(PIT_IRQ);

    /* use the first PIT interrupt to start the first shell */
    if (scheduled_processes[0] == 0) {
        scheduled_processes[0] = 1;
        video_mem = (char*) video_mem_physical[0];
        // current_running_idx = (current_running_idx + 1) % 3;
        /*---------------------------DEBUG-------------------------------*/

        next_kesp = KERNEL_STACK - 4;
        next_kebp = KERNEL_STACK - 4;
        // scheduler_vidmem_remap(0, 0);

        // asm volatile(
        //     "movl %%esp, %0;"
        //     "movl %%ebp, %1;"
        //     : "=r" (cur_kesp), "=r" (cur_kebp)
        // );
        //
        // cur_pcb->kesp = cur_kesp;
        // cur_pcb->kebp = cur_kebp;

        asm volatile(
            "movl %0, %%esp;"
            "movl %1, %%ebp;"
            :
            : "r" (next_kesp), "r" (next_kebp)

        );
        /*---------------------------DEBUG-------------------------------*/
        system_execute((uint8_t*)"shell");
        return 0;
    }

    /* Extract current esp and ebp and save them to pcb */
    asm volatile(
        "movl %%esp, %0;"
        "movl %%ebp, %1;"
        : "=r" (cur_kesp), "=r" (cur_kebp)
    );

    cur_pcb = (pcb_t*)(KERNEL_STACK - (scheduled_processes[current_running_idx]) * PROCESS_CONTROL_BLOCK);
    cur_pcb->kesp = cur_kesp;
    cur_pcb->kebp = cur_kebp;

    /* Update the active schedule index */
    next_running_idx = (current_running_idx + 1) % 3;
    video_mem = (char*) video_mem_physical[next_running_idx];

    /* start the 2nd and 3rd shell */
    if (scheduled_processes[next_running_idx] == 0) {
        scheduled_processes[next_running_idx] = scheduled_processes[current_running_idx] + 1;

        current_running_idx = next_running_idx;
        cursor_id = current_running_idx;
        // if (current_running_idx == current_terminal_id) {
        //     // video_mem = (char*)VIDEO_MEM_ADDR;
        // } else {
        //     video_mem = (char*)video_mem_physical[current_running_idx];
        // }
        video_mem = (char*)video_mem_physical[current_running_idx];

        next_kesp = KERNEL_STACK - (scheduled_processes[next_running_idx] - 1) * PROCESS_CONTROL_BLOCK - 4;
        next_kebp = KERNEL_STACK - (scheduled_processes[next_running_idx] - 1) * PROCESS_CONTROL_BLOCK - 4;
        scheduler_vidmem_remap(next_running_idx, 0);

        // asm volatile(
        //     "movl %%esp, %0;"
        //     "movl %%ebp, %1;"
        //     : "=r" (cur_kesp), "=r" (cur_kebp)
        // );
        //
        // cur_pcb->kesp = cur_kesp;
        // cur_pcb->kebp = cur_kebp;

        asm volatile(
            "movl %0, %%esp;"
            "movl %1, %%ebp;"
            :
            : "r" (next_kesp), "r" (next_kebp)

        );

        // next_pcb = (pcb_t*)(KERNEL_STACK - (scheduled_processes[next_running_idx]) * PROCESS_CONTROL_BLOCK);

        system_execute((uint8_t*)"shell");

        return 0;
    }

    /* General cases for all subsequence scheduling */
    current_running_idx = next_running_idx;
    cursor_id = current_running_idx;
    // if (current_running_idx == current_terminal_id) {
    //     video_mem = (char*)VIDEO_MEM_ADDR;
    // } else {
    //     video_mem = (char*)video_mem_physical[current_running_idx];
    // }
    video_mem = (char*)video_mem_physical[current_running_idx];
    next_pcb = (pcb_t*)(KERNEL_STACK - (scheduled_processes[next_running_idx]) * PROCESS_CONTROL_BLOCK);
    current_pcb = next_pcb;

    //Prepare for context switch
    tss.ss0 = KERNEL_DS;
    // tss.esp0 = next_pcb->kesp;
    tss.esp0 = KERNEL_STACK - (scheduled_processes[next_running_idx] - 1) * PROCESS_CONTROL_BLOCK - 4;

    /*----------------------DEBUG-----------------------*/
    if (scheduled_processes[0] == 4 && cur_pcb->kesp <= 0x7f8f00) {
        cur_pcb->kesp = 0;
    }
    /*----------------------DEBUG-----------------------*/
    // serve as flush TLB
    set_process_pde(PROGRAM_VIRTUAL_START, next_pcb->process_num);
    scheduler_vidmem_remap(next_running_idx, next_pcb->vidmap_flag);
    // printf("test %d \n", current_running_idx);
    // asm volatile(
    //     "movl %%esp, %0;"
    //     "movl %%ebp, %1;"
    //     : "=r" (cur_kesp), "=r" (cur_kebp)
    // );
    // // cur_pcb = (pcb_t*)(KERNEL_STACK - (scheduled_processes[current_running_idx]) * PROCESS_CONTROL_BLOCK);
    // cur_pcb->kesp = cur_kesp;
    // cur_pcb->kebp = cur_kebp;

    asm volatile(
        "movl %0, %%esp;"
        "movl %1, %%ebp;"
        :
        : "r" (next_pcb->kesp), "r" (next_pcb->kebp)

    );

    sti();

    return 0;
}

/*   pit_init
 *   Inputs: none
 *   Return Value: none
 *   Function: Initialize the PIT interrupt
 */
void pit_init(void) {

  cli();

  /* choose PIT mode 3 */
  outb(PIT_MODE_3, PIT_COMMAND);

  /* Send Frequency to Data port */
  outb(FREQ_MASK & FREQUENCY, PIT_CHANNEL_0);
  outb(FREQUENCY >> 8, PIT_CHANNEL_0);

  /* enable IRQ */
  enable_irq(PIT_IRQ);

  sti();

  return;
}
