/* sys_execute.c - Functions to interact with the keyboard
 * vim:ts=4 noexpandtab
 */
#include "systemcall_function.h"
#include "x86_desc.h"
#include "lib.h"
#include "paging.h"
#include "scheduler.h"

#define KERNEL_STACK 0x800000
#define PROCESS_SEGMENT 0x2000
#define PCB_ADDR_MASK 0xFFFFE000
#define PROGRAM_VIRTUAL_START 0x08000000
#define PROGRAM_LOAD_ADDR 0x08048000
#define PAGE_SIZE 0x0400000
#define PROGRAM_VIRTUAL_END PROGRAM_VIRTUAL_START + PAGE_SIZE - 4
#define HEADER_INFO 40
#define FD_ARR_SIZE 8
#define COMMAND_LENGTH 128
/* Segment selector values */
#define KERNEL_CS   0x0010      //00010000
#define KERNEL_DS   0x0018      //00011000
#define USER_CS     0x0023      //00100011
#define USER_DS     0x002B      //00101011
#define KERNEL_TSS  0x0030
#define KERNEL_LDT  0x0038
#define ENTRY_POINT_LOC 24
#define MAXIMUM_PROCESS 6

/* Process number counter */
uint32_t process_total = 0;
/* Input Arguments */
uint8_t argument[COMMAND_LENGTH];
/* Magic numbers for executable identification */
uint8_t magic_num[4] = {0x7f, 0x45, 0x4c, 0x46};

uint8_t process_slots[MAXIMUM_PROCESS];

/*---------------------- local help functions ----------------------*/

/*   sys_execute_parse
 *   Inputs: arg -- the plain text command
 *   Return Value: the length of the command
 *   Function: transfer the text input into executable arguments
 */
int sys_execute_parse(unsigned char* arg) {
    char buf[COMMAND_LENGTH + 1];
    int arg_len;
    int buf_len = 0;
    int pgm=0;              //to indicate whether we meet the first word
    for (arg_len = 0; arg_len < strlen((char*)arg); arg_len++) {
        if (arg[arg_len] == ' ' && pgm ==0)     //spaces at the beginning
            continue;
        else if (arg[arg_len] == ' ' && pgm ==1)    //spaces in the middle
            break;
        else if (arg[arg_len] != ' '){
          pgm=1;
          buf[buf_len] = arg[arg_len];
          buf_len++;
          if (arg[arg_len] == '\0' || arg[arg_len] == '\n')     //command of only one word
              break;
        }
    }
    buf[buf_len] = '\0';

    char argum[COMMAND_LENGTH - arg_len-1];
    int argum_idx=0;                      //index of character in argum
    while(arg_len < strlen((char*)arg)){
        if(arg[arg_len]==' '){            //drop all the spaces
            arg_len++;
            continue;
        }
        else{
            argum[argum_idx]=arg[arg_len];
            arg_len++;
            argum_idx++;
            if (arg[arg_len] == '\0' || arg[arg_len] == '\n')     //if reach the end of command
                break;
        }
    }
    argum[argum_idx] = '\0';

    strcpy((int8_t*)argument,(int8_t*)argum);       //put arguments in a temporary buffer
    strcpy((int8_t*)arg, (int8_t*)buf);             //first word of commmand
    return buf_len;
}
/*---------------------- local help functions ----------------------*/

/*   system_execute (defined in systemcall_function.h)
 *   Inputs: arg -- the command
 *   Return Value: the status of the process (a number between 0 and 255,
 *                 provided by the sys_halt() function)
 *   Function: create a new process, load execute files, and prepare for
 *             context switch
 */
int32_t system_execute(const uint8_t* command){

    cli();

    uint8_t header[HEADER_INFO + 1];
    uint8_t arg[COMMAND_LENGTH + 1];
    dentry_t dentry;
    int32_t eip, cnt, count, rval;
    int current_esp;
    int current_ebp;
    pcb_t* parent_pcb;
    pcb_t* child_pcb;
    uint8_t* load_pos;

    //Parse args
    strcpy((int8_t*)arg, (int8_t*)command);
    sys_execute_parse(arg);

    //Open executable and read file header
    if (-1 == read_dentry_by_name(arg, &dentry) ||
        HEADER_INFO != read_data(dentry.inode_num, 0, header, HEADER_INFO)){
        return -1;
    }

    //Check magic number
    if (strncmp((int8_t*)header, (int8_t*)magic_num, 4)){
        return -1;
    }

    if (process_total >= MAXIMUM_PROCESS) {
        printf("No more process allowed!\n");
        return 0;
    }
    //Get entry point to the file
    memcpy(&eip, header + ENTRY_POINT_LOC, sizeof(int32_t));

    //Setup process paging
    set_process_pde(PROGRAM_VIRTUAL_START, process_total + 1);

    //Load file to 128MB virtual memory
    load_pos = (uint8_t*)PROGRAM_LOAD_ADDR;
    count = 0;
    while (PAGE_SIZE - count > 0) {
        cnt = read_data(dentry.inode_num, count, load_pos + count, PAGE_SIZE - count);
        if (cnt != -1) {
            count += cnt;
        } else
            break;
    }

    //Create Process Control Block
    if (process_total < 3) {
        if (process_total == 0) {
            for (cnt = 0; cnt < MAXIMUM_PROCESS; cnt++){
                process_slots[cnt] = 0;
            }
        }
        asm ("movl %%esp, %0;"
             "movl %%ebp, %1;"
            :"=r" (current_esp), "=r" (current_ebp)
        );
        //No parent process
        child_pcb = (pcb_t*)(KERNEL_STACK - (process_total + 1) * PROCESS_SEGMENT);
        child_pcb->parent_process_num = process_total + 1; // process number starts from 1
        child_pcb->process_num = process_total + 1;
        child_pcb->parent_pcb = child_pcb;
        child_pcb->vidmap_flag = 0;
        child_pcb->kesp = current_esp;
        child_pcb->kebp = current_ebp;
        process_slots[child_pcb->process_num - 1] = 1;
        scheduled_processes[process_total] = child_pcb->process_num;
    }
    else {
        // Acquire avalible process slot
        for (cnt = 0; cnt < MAXIMUM_PROCESS; cnt++){
            if (process_slots[cnt] == 0) {
                break;
            }
        }
        //Acquire current stack pointers
        asm ("movl %%esp, %0;"
             "movl %%ebp, %1;"
            :"=r" (current_esp), "=r" (current_ebp)
        );
        parent_pcb = (pcb_t*)(current_esp & PCB_ADDR_MASK);
        child_pcb = (pcb_t*)(KERNEL_STACK - (cnt + 1) * PROCESS_SEGMENT);
        child_pcb->parent_process_num = parent_pcb->process_num;
        // child_pcb->process_num = parent_pcb->process_num + 1;
        child_pcb->process_num = cnt + 1; //TODO
        child_pcb->parent_pcb = parent_pcb;
        child_pcb->parent_kesp = current_esp;
        child_pcb->parent_kebp = current_ebp;
        child_pcb->vidmap_flag = 0;
        child_pcb->kesp = KERNEL_STACK - (child_pcb->process_num - 1) * PROCESS_SEGMENT - 4;
        child_pcb->kebp = KERNEL_STACK - (child_pcb->process_num - 1) * PROCESS_SEGMENT - 4;
        process_slots[child_pcb->process_num - 1] = 1;
        scheduled_processes[current_running_idx] = child_pcb->process_num;
    }



    strcpy((int8_t*)child_pcb->argbuf, (int8_t*)argument);          //put argument into argbuf in pcb
    process_total++;
    current_pcb = child_pcb;

    //Initialize fd array
    system_open((uint8_t*)"stdin");
    system_open((uint8_t*)"stdout");

    //Prepare for context switch
    tss.ss0 = KERNEL_DS;
    tss.esp0 = current_pcb->kesp;
    // tss.esp0 = KERNEL_STACK - PROCESS_SEGMENT * (current_pcb->process_num - 1) - 4;

    asm volatile(
        "mov $0x2B, %%ax;"
        "mov %%ax, %%ds;"
        "movl $0x83FFFFC, %%eax;"
        "pushl $0x2B;"
        "pushl %%eax;"
        "pushfl;"
        "popl %%edx;"
        "orl $0x200, %%edx;"
        "pushl %%edx;"
        "pushl $0x23;"
        "pushl %1;"
        "iret;"
        "execute_return:;"
        "movl %%eax, %0;"
        : "=g" (rval)
        : "r" (eip)
        : "edx", "eax"
    );
    return rval;
}

/*   system_halt (defined in systemcall_function.h)
 *   Inputs: status -- A specified number for parent execute to return,
 *                     indicateing the status of this process.
 *   Return Value: none, since this function never actually returns
 *   Function: halt the process, clear fds and pcbs, prepare context
 *             for context switch
 */
int32_t system_halt(uint32_t status){

    cli();

    int32_t i;
    uint32_t parent_kebp, parent_kesp, parent_process_num;

    //Prevent from exiting first shell
    if (current_pcb->process_num > 3)
        process_total--;
    else {
        process_total--;
        system_execute((uint8_t*)"shell");
    }
    // printf("Process number %d\n", process_total + 1);
    // process_total--;
    parent_process_num = current_pcb->parent_process_num;
    parent_kebp = current_pcb->parent_kebp;
    parent_kesp = current_pcb->parent_kesp;
    //Restore parent data
    // tss.esp0 = current_pcb->parent_kesp;
    tss.esp0 = KERNEL_STACK - (parent_process_num - 1) * PROCESS_SEGMENT - 4;
    tss.ss0 = KERNEL_DS;
    //Restore parent paging
    set_process_pde(PROGRAM_VIRTUAL_START, parent_process_num);
    //Clear file descriptor array
    // for (i = 0; i < FD_ARR_SIZE; i++){
    //     current_pcb->fd_array[i].flags = 0;
    // }
    for (i = 0; i < FD_ARR_SIZE; i++){
      system_close(i);
    }
    
    process_slots[current_pcb->process_num - 1] = 0;
    current_pcb = current_pcb->parent_pcb;
    scheduled_processes[current_running_idx] = parent_process_num;
    // printf("process %d\n", process_total);
    //Restore parent ebp and esp, return to the parent process
    asm volatile(
        "movl %2, %%eax;"
        "movl %0, %%ebp;"
        "movl %1, %%esp;"
        "sti;"
        "jmp execute_return;"
        :
        : "r" (parent_kebp), "r" (parent_kesp), "r" ((uint32_t)status)
        : "eax", "ebp", "esp"
    );
    return (uint32_t) status;
}
