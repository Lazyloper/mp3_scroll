/* keyboard.c - Functions to interact with the keyboard
 * vim:ts=4 noexpandtab
 */
#include "i8259.h"
#include "keyboard.h"
#include "lib.h"
#include "tests.h"
#include "terminal.h"
#include "isr.h"

#define VIDEO 0xB8000 /* the start of the video memory */
#define KEYMAP_SIZE 69
static uint8_t* vid_ptr = (uint8_t* )VIDEO;

uint8_t input_buffer[MAX_INPUT_LENGTH];
int input_location = 0; /* the current location of the input_buffer */
int new_lines[MAX_INPUT_LENGTH];
int new_lines_location = 0; /* the current location of the nuline_buffer */

static int key_flags[4] = {UP, UP, UP, UP}; /* 4 is the number of flags */

/* local functions */
void map_keys(unsigned char key);
void handle_backspcae();
void set_video_mem(int x, int y, unsigned char c);
/* The four maps transolate the input keycode to characters according to different Control keys */
unsigned char keyboard_map[4][KEYMAP_SIZE] = {
    {
        0,  27, '1', '2', '3', '4', '5', '6', '7', '8',     /* 9 */
      '9', '0', '-', '=', '\b',     /* Backspace */
     '\t',                 /* Tab */
      'q', 'w', 'e', 'r',   /* 19 */
      't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', /* Enter key */
        0,                  /* 29   - Control */
      'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',     /* 39 */
     '\'', '`',   0,                /* Left shift */
     '\\', 'z', 'x', 'c', 'v', 'b', 'n',                    /* 49 */
      'm', ',', '.', '/',   0,                              /* Right shift */
      '*',
        0,  /* Alt */
      ' ',  /* Space bar */
        0,  /* Caps lock */
        'Z',  /* 59 - F1 key ... > */
        'X',   'C',   0,   0,   0,   0,   0,   0,
        0,  /* < ... F10 */
    },

    {
        0,  27, '!', '@', '#', '$', '%', '^', '&', '*',     /* 9 */
      '(', ')', '_', '+', '\b',     /* Backspace */
     '\t',                 /* Tab */
      'Q', 'W', 'E', 'R',   /* 19 */
      'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', /* Enter key */
        0,                  /* 29   - Control */
      'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',     /* 39 */
     '\"', '~',   0,                /* Left shift */
      '|', 'Z', 'X', 'C', 'V', 'B', 'N',                    /* 49 */
      'M', '<', '>', '?',   0,                              /* Right shift */
      '*',
        0,  /* Alt */
      ' ',  /* Space bar */
        0,  /* Caps lock */
        0,  /* 59 - F1 key ... > */
        0,   0,   0,   0,   0,   0,   0,   0,
        0,  /* < ... F10 */
    },

    {
        0,  27, '1', '2', '3', '4', '5', '6', '7', '8',     /* 9 */
      '9', '0', '-', '=', '\b',     /* Backspace */
      '\t',                 /* Tab */
      'Q', 'W', 'E', 'R',   /* 19 */
      'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n', /* Enter key */
        0,                  /* 29   - Control */
      'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';',     /* 39 */
     '\'', '`',   0,                /* Left shift */
     '|', 'Z', 'X', 'C', 'V', 'B', 'N',                    /* 49 */
      'M', ',', '.', '/',   0,                              /* Right shift */
      '*',
        0,  /* Alt */
      ' ',  /* Space bar */
        0,  /* Caps lock */
        0,  /* 59 - F1 key ... > */
        0,   0,   0,   0,   0,   0,   0,   0,
        0,  /* < ... F10 */
    },

    {
        0,  27, '!', '@', '#', '$', '%', '^', '&', '*',     /* 9 */
      '(', ')', '_', '+', '\b',     /* Backspace */
     '\t',                 /* Tab */
      'q', 'w', 'r', 't',   /* 19 */
      't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\n', /* Enter key */
        0,                  /* 29   - Control */
      'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':',     /* 39 */
     '\"', '~',   0,                /* Left shift */
      '|', 'z', 'x', 'c', 'v', 'b', 'n',                    /* 49 */
      'm', '<', '>', '?',   0,                              /* Right shift */
      '*',
        0,  /* Alt */
      ' ',  /* Space bar */
        0,  /* Caps lock */
        0,  /* 59 - F1 key ... > */
        0,   0,   0,   0,   0,   0,   0,   0,
        0,  /* < ... F10 */
    },
};

/*   keyboard_init
 *   Inputs: none
 *   Return Value: none
 *   Function: Initialize the keyboard input (enable the corresponding IRQ busline)
 */
void keyboard_init(void) {
    enable_irq(KEYBOARD_IRQ); // 1 -- the irq_num of keyboard
    // enable_cursor(0, 25);
}

/*   keyboard_interrupt_handler
 *   Inputs: none
 *   Return Value: none
 *   Function: called when a keyboard interrupt is raised
 */
void keyboard_interrupt_handler(void) {
    //printf("keyboard reached\n");
    cli();

    uint8_t status;
    unsigned char key;

    /* Acknowlegement */
    status = inb(KB_COMMAND);

    /* Examine the last bit of status */
    if (status & 0x01) {
        /* Read input keycode */
        key = inb(KB_DATA);
        /* end of the interrupt */
        send_eoi(KEYBOARD_IRQ);
        /* Only print characters on keydown event that have
         * a non-zero mapping */
        switch (key) {
            case LSHIFT_DOWN:
                key_flags[SHIFT] = DOWN;
                break;
            case LSHIFT_UP:
                key_flags[SHIFT] = UP;
                break;
            case LCTRL_DOWN:
                key_flags[CTRL] = DOWN;
                break;
            case LCTRL_UP:
                key_flags[CTRL] = UP;
                break;
            case LALT_DOWN:
                key_flags[ALT] = DOWN;
                break;
            case LALT_UP:
                key_flags[ALT] = UP;
                break;
            case CAPS_DOWN:
                key_flags[CAPS] = DOWN - key_flags[CAPS];
                break;
            case BACKSPACE:
                handle_backspcae();
                break;
            default:
                map_keys(key);
                break;
        }
    }

    /* end of the interrupt */
    send_eoi(KEYBOARD_IRQ);

    sti();
}

/*   map_keys
 *   Inputs: key -- the scancode
 *   Return Value: none
 *   Function: transfer the scancode into keyvalues
 */
void map_keys(unsigned char key) {
    char* temp_vidmem;
    int temp_cursor_id;
    if (key > KEYMAP_SIZE) return; /* return if the key cannot be handled */

    int index = 0; /* The index of the keymap that will be used */
    unsigned char character;

    temp_vidmem = video_mem;
    video_mem = (char*)0xB8000;
    temp_cursor_id = cursor_id;
    cursor_id = current_terminal_id;

    /* decide which map to use base on functional keys */
    if (key_flags[CTRL] == DOWN || key_flags[ALT] == DOWN) {
        index = 0; /* if control or alt is being pressed, the first map will be used. */
    }
    else {
        index = (key_flags[CAPS] << 1) + key_flags[SHIFT]; /* the combnition of flag CAPS and flag SHIFT form a 2-bit number */
    }

    character = keyboard_map[index][key];

    if(key_flags[ALT] == DOWN){
      switch (character){
        case 'Z':
          // printf("switch to terminal 1!\n");
          switch_terminals(2);
          break;
        case 'X':
          // printf("switch to terminal 2!\n");
          switch_terminals(1);
          break;
        case 'C':
          // printf("switch to terminal 3!\n");
          switch_terminals(0);
          break;
        // case '`':
        //   intr_20();
        //   break;
        default:
          break;
      }
      cursor_id = temp_cursor_id;
      video_mem = temp_vidmem;
      return;
    }

    /* Ctrl is pressed */
    if(key_flags[CTRL] == DOWN) {
        switch (character) {
            case 'l':
                // set cursor
                clear();
                update_cursor(terminals[cursor_id].screen_x, terminals[cursor_id].screen_y);
                cursor_id = temp_cursor_id;
                video_mem = temp_vidmem;
                input_location = 0;       /* clear the input_buffer and
                new_lines_location = 0;      new_lines buffer */
                break;
            case 'w':
              scroll_up();
              break;
            case 's':
              scroll_down();
              break;
            default:
                break;
        }
        return;
    }


    if(key < 58 && character) {
        // read_flag = 0;
        //-------------- Output to the screen ----------------
        // vid_ptr[current_loc++] = character;
        // /* Attribute 0x07 is white font on black background */
        // vid_ptr[current_loc++] = 0x07;
        // /* check if the current input is a new line */
        //-------------- Output to the screen ----------------

        if(character == '\n') {
            /* fill the new_lines buffer */
            new_lines[new_lines_location++] = input_location;
            // if (current_row + 1 == NUM_ROWS)
            //     scroll_up();
            // else
            // set_video_mem(screen_x, screen_y, ' ');
            putc(character);
            video_mem = temp_vidmem;
            cursor_id = temp_cursor_id;
            // update_cursor(screen_x, screen_y);
            // terminal_read_test1();
            // terminal_read_test2();
            terminals[current_terminal_id].read_flag = READ_ENABLE;
            // terminal_read(0, read_buffer, MAX_INPUT_LENGTH);
            return;
        }
        /* check if the buffer is full */
        else if(input_location < MAX_INPUT_LENGTH - 1) {
            /* Fill the input buffer */
            terminals[current_terminal_id].read_flag = READ_DISABLE;
            input_buffer[input_location++] = character;
            putc(character);
            video_mem = temp_vidmem;
            cursor_id = temp_cursor_id;
            return;
        }
        terminals[current_terminal_id].read_flag = READ_DISABLE;
        // terminal_read(0, read_buffer, MAX_INPUT_LENGTH);
    }
}

/*   handle_backspcae
 *   Inputs: none
 *   Return Value: none
 *   Function: handle the backspace
 */
void handle_backspcae(void) {
    int temp_cursor_id, temp_vidmem;
    temp_cursor_id = cursor_id;
    cursor_id = current_terminal_id;
    temp_vidmem = (int)video_mem;
    video_mem = (char*)VIDEO;
    int line_start;
    if (new_lines_location == 0)
        line_start = 0; /* store the starting position of the current line */
    else
        line_start = new_lines[new_lines_location-1];
    if (input_location > line_start) {
        if (terminals[cursor_id].screen_x != 0) terminals[cursor_id].screen_x--;
        else {
            terminals[cursor_id].screen_y--;
            terminals[cursor_id].screen_x = NUM_COLS-1;
        }
        input_location--;
    }
    set_video_mem(terminals[cursor_id].screen_x, terminals[cursor_id].screen_y, '\0');
    update_cursor(terminals[cursor_id].screen_x, terminals[cursor_id].screen_y);
    cursor_id = temp_cursor_id;
    video_mem = (char*)temp_vidmem;
}

/*   set_video_mem
 *   Inputs: key -- the scancode
 *   Return Value: none
 *   Function: set some value into specified video memory location
 */
void set_video_mem(int x, int y, unsigned char c) {
    vid_ptr[2*(y*NUM_COLS+x)] = c;
    vid_ptr[2*(y*NUM_COLS+x)+1] = 0x07; // set background color on screen
}
