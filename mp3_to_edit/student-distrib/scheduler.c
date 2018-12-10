#include "scheduler.h"
#include "systemcall_function.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "paging.h"
#include "terminal.h"


#define KERNEL_STACK 0x800000
#define PROCESS_CONTROL_BLOCK 0x2000
#define PROGRAM_VIRTUAL_START 0x08000000

/* Segment selector values */
#define KERNEL_DS   0x0018      //00011000


uint32_t scheduled_processes[3] = {0, 0, 0};

uint32_t current_running_idx = 0;

int32_t scheduling() {
    cli();

    pcb_t *next_pcb, *cur_pcb;
    uint32_t next_running_idx;
    int32_t cur_kesp, cur_kebp, next_kesp, next_kebp;

    send_eoi(PIT_IRQ);
    video_mem = (char*) VIDEO_MEM_BUF;
    if (scheduled_processes[0] == 0) {
        scheduled_processes[0] = 1;
        // current_running_idx = (current_running_idx + 1) % 3;
        sti();
        system_execute((uint8_t*)"shell");
        return 0;
    }

    asm volatile(
        "movl %%esp, %0;"
        "movl %%ebp, %1;"
        : "=r" (cur_kesp), "=r" (cur_kebp)
    );

    cur_pcb = (pcb_t*)(KERNEL_STACK - (scheduled_processes[current_running_idx]) * PROCESS_CONTROL_BLOCK);
    cur_pcb->kesp = cur_kesp;
    cur_pcb->kebp = cur_kebp;

    next_running_idx = (current_running_idx + 1) % 3;

    // scheduler_vidmem_remap(next_running_idx + 1, 1);

    if (scheduled_processes[next_running_idx] == 0) {

        scheduled_processes[next_running_idx] = scheduled_processes[current_running_idx] + 1;

        current_running_idx = next_running_idx;
        cursor_id = current_running_idx;
        if (current_running_idx == current_terminal_id) {
            video_mem = (char*)VIDEO_MEM_ADDR;
        } else {
            video_mem = (char*)VIDEO_MEM_BUF;
        }

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
        sti();

        system_execute((uint8_t*)"shell");

        return 0;
    }

    current_running_idx = next_running_idx;
    cursor_id = current_running_idx;
    if (current_running_idx == current_terminal_id) {
        video_mem = (char*)VIDEO_MEM_ADDR;
    } else {
        video_mem = (char*)VIDEO_MEM_BUF;
    }
    next_pcb = (pcb_t*)(KERNEL_STACK - (scheduled_processes[next_running_idx]) * PROCESS_CONTROL_BLOCK);
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

void pit_init(void) {

  cli();

  outb(PIT_MODE_3, PIT_COMMAND);
  outb(FREQ_MASK & FREQUENCY, PIT_CHANNEL_0);
  outb(FREQ_MASK >> 8, PIT_CHANNEL_0);

  enable_irq(PIT_IRQ);

  sti();

  return;
}
