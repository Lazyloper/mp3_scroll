/* terminal.h - Defines used in interactions with the terminal
 * vim:ts=4 noexpandtab
 */
#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "types.h"
#include "lib.h"

/* Externally-visible functions */

#define MAX_INPUT_LENGTH 128
#define TERMINAL_NUMBER 3

typedef struct{
  uint8_t terminal_id;

  uint32_t screen_x;
  uint32_t screen_y;

  uint8_t launch_flag;    // 0 means this terminal is not launched and 1 means it is launched
  uint32_t video_mem;

  int32_t freq;
  int32_t clap;

  uint8_t input_buffer[MAX_INPUT_LENGTH];
  int input_location;
  int new_lines[MAX_INPUT_LENGTH];
  int new_lines_location;
  volatile int read_flag;
}term_t;

term_t terminals[TERMINAL_NUMBER];
uint32_t current_terminal_id;

void init_terminal();

// int32_t launch_terminal(uint8_t id);

int32_t save_state(uint8_t id);

int32_t restore_state(uint8_t id);

int32_t switch_terminals(uint8_t id);

int32_t terminal_open(const uint8_t* filename);

int32_t terminal_close(int32_t fd);

int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);

int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);

#endif
