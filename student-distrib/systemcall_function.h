#ifndef SYSTEMCALL_FUNCTION_H
#define SYSTEMCALL_FUNCTION_H

#include "types.h"
#include "systemcall_function.h"
#include "lib.h"
#include "rtc.h"
#include "terminal.h"
#include "filesyshelper.h"

#define MAX_FILES_NUMBER 8
#define NINUSE  0x0000
#define INUSE   0x0001
#define FILE_START  0x0000

typedef struct files_operation{
    int32_t (*open)(const uint8_t* filename);
    int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
    int32_t (*close)(int32_t fd);
}files_operation_t;

typedef struct fd {
  files_operation_t file_operations_pointer;
  int32_t inode;
  uint32_t file_position;
  uint32_t flags;
} fd_t;

typedef struct pcb {
    fd_t fd_array[8];
    uint32_t parent_kesp;
    uint32_t parent_kebp;
    uint32_t kesp;
    uint32_t kebp;
    uint32_t process_num;
    uint32_t parent_process_num;
    uint32_t vidmap_flag;
    struct pcb* parent_pcb;
    int8_t  argbuf[128];
} pcb_t;

extern pcb_t* current_pcb;

extern int32_t system_halt(uint32_t status);

extern int32_t system_execute (const uint8_t* command);

int32_t system_read (int32_t fd, void* buf, int32_t nbytes);

int32_t system_write (int32_t fd, const void* buf, int32_t nbytes);

int32_t system_open (const uint8_t* filename);

int32_t system_close (int32_t fd);

int32_t system_getargs(uint8_t* buf, int32_t nbytes);

int32_t system_vidmap(uint8_t** screen_start);

int32_t system_set_handler (int32_t signum, void* handler_address);

int32_t system_sigreturn (void);

int32_t fake_terminal_write(int32_t fd, const void* buf, int32_t nbytes);

int32_t fake_terminal_read(int32_t fd, void* buf, int32_t nbytes);

// int32_t stdin_operations_table[4] = {(int32_t) terminal_open,
//                                       (int32_t) terminal_read,
//                                       (int32_t) terminal_write,
//                                       (int32_t) terminal_close
//                                     };
// int32_t stdout_operations_table[4] = {(int32_t) terminal_open,
//                                       (int32_t) terminal_read,
//                                       (int32_t) terminal_write,
//                                       (int32_t) terminal_close
//                                     };
// int32_t rtc_operations_table[4] = {(int32_t) rtc_open,
//                                       (int32_t) rtc_read,
//                                       (int32_t) rtc_write,
//                                       (int32_t) rtc_close
//                                     };
// int32_t dir_operations_table[4] = {(int32_t) dir_open,
//                                     (int32_t) dir_read,
//                                     (int32_t) dir_write,
//                                     (int32_t) dir_close
//                                     };
// int32_t file_operations_table[4] = {(int32_t) file_open,
//                                     (int32_t) file_read,
//                                     (int32_t) file_write,
//                                     (int32_t) file_close
//                                     };

#endif
