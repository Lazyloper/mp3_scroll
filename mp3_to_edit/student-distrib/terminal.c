/* terminal.c - Functions to interact with the terminal
 * vim:ts=4 noexpandtab
 */

#include "keyboard.h"
#include "lib.h"
#include "terminal.h"
#include "paging.h"
#include "scheduler.h"
// #include "terminal.h"

#define TERMINAL_NUMBER 3
term_t terminals[TERMINAL_NUMBER];
int8_t current_terminal_id;


int32_t terminal_vidmem_remap(uint8_t id){
  if(id>=3){return -1;}
  // if(check_bit == 0){
  int32_t addr;
  addr = terminals[id].video_mem;
  page_tbl[(VIDEO_MEM_BUF & MASK_TABLE) >> 12].page_base_addr = terminals[id].video_mem >> 12;
  page_tbl[(VIDEO_MEM_BUF & MASK_TABLE) >> 12].present = 1;
  page_tbl[(VIDEO_MEM_BUF & MASK_TABLE) >> 12].read_write = 1;
  page_tbl[(VIDEO_MEM_BUF & MASK_TABLE) >> 12].user_supervisor = 1;
  // }
  // else if(check_bit == 1){
  //   page_tbl[(terminals[id].video_mem & MASK_TABLE) >> 12].page_base_addr = (video_mem_phyiscal[i]) >> 12;
  //   page_tbl[(terminals[id].video_mem & MASK_TABLE) >> 12].present = 1;
  //   page_tbl[(terminals[id].video_mem & MASK_TABLE) >> 12].read_write = 1;
  //   page_tbl[(terminals[id].video_mem & MASK_TABLE) >> 12].user_supervisor = 1;
  // }
  //flush TLB
  flush_tlb();
  return 0;
}

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
  current_terminal_id = 0;

  //launch the first terminal

  // execute((uint8_t*)"shell");
}


//load the state in terminals[id] into current terminal
int32_t restore_state(uint8_t id){
  current_terminal_id = id;
  input_location = terminals[id].input_location;
  new_lines_location = terminals[id].new_lines_location;
  memcpy((uint8_t*)input_buffer, (uint8_t*)terminals[id].input_buffer, MAX_INPUT_LENGTH);
  memcpy((uint8_t*)new_lines, (uint8_t*)terminals[id].new_lines, MAX_INPUT_LENGTH);
  update_cursor(terminals[id].screen_x, terminals[id].screen_y);

  terminal_vidmem_remap(id);
  memcpy((uint8_t*)VIDEO_MEM_ADDR, (uint8_t*)VIDEO_MEM_BUF, NUM_COLS*NUM_ROWS*2);

  return 0;
}

//save the state of the current terminals into terminals[id]
int32_t save_state(uint8_t id){
  terminals[id].input_location = input_location;
  terminals[id].new_lines_location = new_lines_location;
  memcpy((uint8_t*)terminals[id].input_buffer,(uint8_t*)input_buffer, MAX_INPUT_LENGTH);
  memcpy((uint8_t*)terminals[id].new_lines,(uint8_t*)new_lines, MAX_INPUT_LENGTH);

  //@TODO: //how to get the corsor
  // terminals[id].screen_x = screen_x;
  // terminals[id].screen_y = screen_y;

  terminal_vidmem_remap(id);
  memcpy((uint8_t*)VIDEO_MEM_BUF, (uint8_t*)VIDEO_MEM_ADDR, NUM_COLS*NUM_ROWS*2);
  return 0;
}

int32_t switch_terminals(uint8_t id){
  if(id == current_terminal_id){return 0;}
  if(id >= TERMINAL_NUMBER){return -1;}
  save_state(current_terminal_id);
  restore_state(id);
  terminal_vidmem_remap(current_running_idx);

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

    cli();
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
            sti();
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
            sti();
            return nbytes;
        }
    }
    /* there has not been a existing line yet */
    else {
        terminals[current_running_idx].read_flag = READ_DISABLE;
        sti();
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
    cli();
    int32_t i = 0;
    buf = (uint8_t*)buf;
    for (i = 0; i < nbytes; i++) {
        putc(((uint8_t*)buf)[i]);
    }
    sti();
    return i;
}
