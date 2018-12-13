/* terminal.c - Functions to interact with the terminal
 * vim:ts=4 noexpandtab
 */

#include "keyboard.h"
#include "lib.h"
#include "terminal.h"
#include "paging.h"
#include "scheduler.h"
#include "systemcall_function.h"
// #include "terminal.h"

#define TERMINAL_NUMBER 3
term_t terminals[TERMINAL_NUMBER];
uint32_t current_terminal_id = 2;


/*terminal_vidmem_remap
 *Description: map the visual memry to the cooreponsed physical memory
 *Inputs:      uint8_t id: the id pf the visual memory of the terminal we want to remap
               int32_t vidmap_flag: check which addr we need to remap
 *Outputs:     Return 0-Success
 *                    1-Failure
 *Effects:     change the paging dir
 */
int32_t terminal_vidmem_remap(uint8_t id, int32_t vidmap_flag){
  if(id>=3){return -1;}
  // if(check_bit == 0){
  int32_t addr;
  addr = terminals[id].video_mem;
  page_tbl[(terminals[id].video_mem & MASK_TABLE) >> 12].page_base_addr = terminals[id].video_mem >> 12;
  page_tbl[(terminals[id].video_mem & MASK_TABLE) >> 12].present = 1;
  page_tbl[(terminals[id].video_mem & MASK_TABLE) >> 12].read_write = 1;
  page_tbl[(terminals[id].video_mem & MASK_TABLE) >> 12].user_supervisor = 1;
  // }
  // else if(check_bit == 1){
  //   page_tbl[(terminals[id].video_mem & MASK_TABLE) >> 12].page_base_addr = (video_mem_phyiscal[i]) >> 12;
  //   page_tbl[(terminals[id].video_mem & MASK_TABLE) >> 12].present = 1;
  //   page_tbl[(terminals[id].video_mem & MASK_TABLE) >> 12].read_write = 1;
  //   page_tbl[(terminals[id].video_mem & MASK_TABLE) >> 12].user_supervisor = 1;
  // }
  //flush TLB
  if (vidmap_flag) {
    //PTE for video memory
    userpage_tbl[0].page_base_addr = addr >> 12;
    userpage_tbl[0].present = 1;
    userpage_tbl[0].read_write = 1;
    userpage_tbl[0].user_supervisor = 1;
  }
  flush_tlb();
  return 0;
}


/*terminal_vidmem_remap
 *Description: map the visual memry to the video memory
 *Inputs:      uint8_t id: the id pf the visual memory of the terminal we want to remap
               int32_t vidmap_flag: check which addr we need to remap
 *Outputs:     Return 0-Success
 *                    1-Failure
 *Effects:     change the paging dir
 */
int32_t terminal_vidmem_map(uint8_t id, int32_t vidmap_flag){
  if(id>=3){return -1;}
  // if(check_bit == 0){
  int32_t addr;
  addr = VIDEO_MEM_ADDR;
  page_tbl[(terminals[id].video_mem & MASK_TABLE) >> 12].page_base_addr = addr >> 12;
  page_tbl[(terminals[id].video_mem & MASK_TABLE) >> 12].present = 1;
  page_tbl[(terminals[id].video_mem & MASK_TABLE) >> 12].read_write = 1;
  page_tbl[(terminals[id].video_mem & MASK_TABLE) >> 12].user_supervisor = 1;
  // }
  // else if(check_bit == 1){
  //   page_tbl[(terminals[id].video_mem & MASK_TABLE) >> 12].page_base_addr = (video_mem_phyiscal[i]) >> 12;
  //   page_tbl[(terminals[id].video_mem & MASK_TABLE) >> 12].present = 1;
  //   page_tbl[(terminals[id].video_mem & MASK_TABLE) >> 12].read_write = 1;
  //   page_tbl[(terminals[id].video_mem & MASK_TABLE) >> 12].user_supervisor = 1;
  // }
  //flush TLB
  if (vidmap_flag) {
    //PTE for video memory
    userpage_tbl[0].page_base_addr = addr >> 12;
    userpage_tbl[0].present = 1;
    userpage_tbl[0].read_write = 1;
    userpage_tbl[0].user_supervisor = 1;
  }
  flush_tlb();
  return 0;
}


/*init_terminal
 *Description: initialize the parameter in the terminal structure
 *Inputs:       none
 *Outputs:     Return 0-Success
 *                    1-Failure
 *Effects:     none
 */
void init_terminal(){
  int i;

  for(i = 0; i < TERMINAL_NUMBER; i++){
    terminals[i].terminal_id = i;
    terminals[i].screen_x = 0;
    terminals[i].screen_y = 0;
    terminals[i].launch_flag = 0;
    terminals[i].input_location = 0;
    terminals[i].new_lines_location = 0;
    terminals[i].video_mem = video_mem_physical[i];
    terminals[i].read_flag = READ_DISABLE;
    terminals[i].freq=2;
    terminals[i].clap=511;
    //@TODO: //initial input_buffer
     // terminal_vidmem_remap(0);

  }
  current_terminal_id = 2;

  //launch the first terminal

  // execute((uint8_t*)"shell");
}

/*restore_state
 *Description: load the state in terminals[id] into current terminal
 *Inputs:       id: the terminal we want to restore
 *Outputs:     Return 0-Success
 *                    1-Failure
 *Effects:     none
 */
int32_t restore_state(uint8_t id){
  pcb_t* current_process;
  current_process = (pcb_t*)(KERNEL_STACK - (scheduled_processes[id]) * PROCESS_CONTROL_BLOCK);
  current_terminal_id = id;
  input_location = terminals[id].input_location;
  new_lines_location = terminals[id].new_lines_location;
  memcpy((uint8_t*)input_buffer, (uint8_t*)terminals[id].input_buffer, MAX_INPUT_LENGTH);
  memcpy((uint8_t*)new_lines, (uint8_t*)terminals[id].new_lines, MAX_INPUT_LENGTH);
  update_cursor(terminals[id].screen_x, terminals[id].screen_y);

  terminal_vidmem_remap(id, current_process->vidmap_flag);
  memcpy((uint8_t*)VIDEO_MEM_ADDR, (uint8_t*)terminals[id].video_mem, NUM_COLS*NUM_ROWS*2);

  return 0;
}

/*save_state
 *Description: save the state of the current terminals into terminals[id]
 *Inputs:       id: the terminal we want to restore
 *Outputs:     Return 0-Success
 *                    1-Failure
 *Effects:     none
 */
int32_t save_state(uint8_t id){
  pcb_t* current_process;
  current_process = (pcb_t*)(KERNEL_STACK - (scheduled_processes[id]) * PROCESS_CONTROL_BLOCK);
  terminals[id].input_location = input_location;
  terminals[id].new_lines_location = new_lines_location;
  memcpy((uint8_t*)terminals[id].input_buffer,(uint8_t*)input_buffer, MAX_INPUT_LENGTH);
  memcpy((uint8_t*)terminals[id].new_lines,(uint8_t*)new_lines, MAX_INPUT_LENGTH);

  //@TODO: //how to get the corsor
  // terminals[id].screen_x = screen_x;
  // terminals[id].screen_y = screen_y;

  terminal_vidmem_remap(id, current_process->vidmap_flag);
  memcpy((uint8_t*)terminals[id].video_mem, (uint8_t*)VIDEO_MEM_ADDR, NUM_COLS*NUM_ROWS*2);
  return 0;
}


/*switch_terminals
 *Description: switch to another terminal
 *Inputs:       id: the terminal we want to switch to
 *Outputs:     Return 0-Success
 *                    1-Failure
 *Effects:     none
 */
int32_t switch_terminals(uint8_t id){
  if(id == current_terminal_id){return 0;}
  if(id >= TERMINAL_NUMBER){return -1;}
  pcb_t* current_process;
  current_process = (pcb_t*)(KERNEL_STACK - (scheduled_processes[id]) * PROCESS_CONTROL_BLOCK);
  save_state(current_terminal_id);
  restore_state(id);
  terminal_vidmem_map(id, current_process->vidmap_flag);

  return 0;
}


/*terminal_open
 *Description: Open the terminal
 *Inputs:      none
 *Outputs:     Return 0-Success
 *                    1-Failure
 *Effects:     Set the frequency of rtc to 2HZ
 */
int32_t terminal_open(const uint8_t* filename) {
    terminals[current_running_idx].read_flag = READ_DISABLE;
    return 0;
}

/*terminal_close
 *Description: close terminal
 *Inputs:      buf- hold the value of frequency to be set
 *             nbytes- need to be 4 to work
 *Outputs:     none
 *Effects:     Set RTC frequency to the value in buf
 */
int32_t terminal_close(int32_t fd) {
    return 0;
}

/*terminal_read
 *Description: Read the keyboard input buffer
 *Inputs:      buf- whatever
 *             nbytes- whatever
 *Outputs:     Return 0-Success
 *                    1-Failure
 *Effects:     Read after interrupt occur and reset the flag
 */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes) {

    int end_position;
    buf = (unsigned char*)buf;
    while (!terminals[current_running_idx].read_flag) {}

    // cli();
    /* if there has been a line in the input buffer currently */
    if (new_lines_location > 0) {
        end_position = new_lines[0];
        /* if the line is shoter than the output buffer size, we copy the entire line */
        if (nbytes > end_position) {
            memcpy(buf, input_buffer, end_position);
            ((unsigned char*)buf)[end_position] = '\n';
            input_location -= end_position;
            memmove(input_buffer, input_buffer + end_position, input_location);
            new_lines_location--;
            memmove(new_lines, new_lines + 1, new_lines_location);
            terminals[current_running_idx].read_flag = READ_DISABLE;
            // sti();
            return end_position + 1;
        }
        /* if the line is longer than the output buffer size, we align it into the buffer */
        else {
            memcpy(buf, input_buffer, nbytes-1);
            ((unsigned char*)buf)[nbytes-1] = '\0';
            input_location -= end_position;
            memmove(input_buffer, input_buffer + end_position, input_location);
            new_lines_location--;
            memmove(new_lines, new_lines + 1, new_lines_location);
            terminals[current_running_idx].read_flag = READ_DISABLE;
            // sti();
            return nbytes;
        }
    }
    /* there has not been a existing line yet */
    else {
        terminals[current_running_idx].read_flag = READ_DISABLE;
        // sti();
        return 0;
    }
}

/*terminal_write
 *Description: output the buffer input
 *Inputs:      buf- hold the value of frequency to be set
 *             nbytes- need to be 4 to work
 *Outputs:     none
 *Effects:     Set RTC frequency to the value in buf
 */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes) {
    // cli();
    int32_t i = 0;
    buf = (uint8_t*)buf;
    for (i = 0; i < nbytes; i++) {
        putc(((uint8_t*)buf)[i]);
    }
    // sti();
    return i;
}
