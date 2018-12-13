/* keyboard.h - Defines used in interactions with the keyboard
 * vim:ts=4 noexpandtab
 */
#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "types.h"

/* Ports and IRQ nums the keyborad sits on */
#define KEYBOARD_IRQ 1
#define KB_DATA 0x60
#define KB_COMMAND 0x64

#define MAX_INPUT_LENGTH 128
#define SHIFT 0
#define CTRL 1
#define ALT 2
#define CAPS 3

#define UP 0
#define DOWN 1

#define LSHIFT_DOWN 0x2A
#define LSHIFT_UP 0xAA
#define LCTRL_DOWN 0x1D
#define LCTRL_UP 0x9D
#define LALT_DOWN 0x38
#define LALT_UP 0xB8
#define CAPS_DOWN 0x3A
#define BACKSPACE 0x0E

#define READ_ENABLE 1
#define READ_DISABLE 0

/* Externally-visible functions */

/* Initialize the keyboard input (enable the corresponding IRQ busline) */
void keyboard_init(void);

/* called when a keyboard interrupt is raised */
extern void keyboard_interrupt_handler(void);

extern uint8_t input_buffer[MAX_INPUT_LENGTH];
extern int input_location; /* the location of the input_buffer */
extern int new_lines[MAX_INPUT_LENGTH];
extern int new_lines_location;
extern volatile int32_t read_flag;

#endif
