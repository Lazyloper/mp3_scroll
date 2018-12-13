#include "systemcall_function.h"
#include "lib.h"
#include "rtc.h"
#include "terminal.h"
#include "filesyshelper.h"
#include "paging.h"

pcb_t* current_pcb = NULL;

#define STDIN_LENGTH 5
#define STDOUT_LENGTH 6
#define RTC_LENGTH 3
#define DIR_LENGTH 1

//Initialize the five type of files_operation

files_operation_t stdin_table =  {terminal_open, terminal_read, fake_terminal_write, terminal_close};
files_operation_t stdout_table =  {terminal_open, fake_terminal_read, terminal_write, terminal_close};
files_operation_t rtc_table =  {rtc_open, rtc_read, rtc_write, rtc_close};
files_operation_t dir_table =  {dir_open, dir_read, dir_write, dir_close};
files_operation_t file_table =  {file_open, file_read, file_write, file_close};


// system_read():
// description: read nbytes from file into the buf
// input : fd : the file descriptor of the file we need to write
//         buf : where the data stored
//         nbytes: the length of data need to read
// output : 0 if success
// side effect : none

int32_t system_read (int32_t fd, void* buf, int32_t nbytes){
  int ret;
  if(current_pcb->fd_array[fd].flags == 0){return -1;}
  if(fd<0 ||fd >= MAX_FILES_NUMBER){return -1;}
  if(buf == NULL){return -1;}
  ret = current_pcb->fd_array[fd].file_operations_pointer.read(fd, buf, nbytes);
  if(ret == -1){return -1;}
  current_pcb->fd_array[fd].file_position += ret;
  return ret;
}


// system_write():
// description: write nbytes from buf into the file
// input : fd : the file descriptor of the file we need to write
//         buf : where the data stored
//         nbytes: the length of data need to write
// output : 0 if success
// side effect : none

int32_t system_write (int32_t fd, const void* buf, int32_t nbytes){
  if(current_pcb->fd_array[fd].flags == 0){return -1;}
  if(fd<0 ||fd >= MAX_FILES_NUMBER){return -1;}
  if(buf == NULL){return -1;}
  if(current_pcb->fd_array[fd].file_operations_pointer.write(fd, buf, nbytes) == -1){return -1;}
  return nbytes;
}


// system_open():
// description: open the file
// input : filename : the name of the file we need to open
// output : 0 if success
//          -1 if fail
// side effect : none

int32_t system_open (const uint8_t* filename){
  int index;
  for (index = 0; index < MAX_FILES_NUMBER; index++){
    if (current_pcb->fd_array[index].flags == 0){break;}
  }
  if(index == MAX_FILES_NUMBER){return -1;}

  if (strlen((int8_t*)filename) == STDIN_LENGTH && !strncmp((int8_t*)filename, "stdin", STDIN_LENGTH)){
    current_pcb->fd_array[index].file_operations_pointer = stdin_table;
    current_pcb->fd_array[index].inode = 0;
    current_pcb->fd_array[index].file_position = 0;
    current_pcb->fd_array[index].flags = 1;
    terminal_open((uint8_t*)"stdin");
  }

  else if (strlen((int8_t*)filename) == STDOUT_LENGTH && !strncmp((int8_t*)filename, "stdout", STDOUT_LENGTH)){
    current_pcb->fd_array[index].file_operations_pointer = stdout_table;
    current_pcb->fd_array[index].inode = 0;
    current_pcb->fd_array[index].file_position = 0;
    current_pcb->fd_array[index].flags = 1;
    terminal_open((uint8_t*)"stdout");
  }

  else if (strlen((int8_t*)filename) == RTC_LENGTH && !strncmp((int8_t*)filename, "rtc", RTC_LENGTH)){
    current_pcb->fd_array[index].file_operations_pointer = rtc_table;
    current_pcb->fd_array[index].inode = 0;
    current_pcb->fd_array[index].file_position = 0;
    current_pcb->fd_array[index].flags = 1;
    rtc_open((uint8_t*)"rtc");
  }

  else if (strlen((int8_t*)filename) == DIR_LENGTH && !strncmp((int8_t*)filename, ".", DIR_LENGTH)){
    current_pcb->fd_array[index].file_operations_pointer = dir_table;
    current_pcb->fd_array[index].inode = 0;
    current_pcb->fd_array[index].file_position = 0;
    current_pcb->fd_array[index].flags = 1;
    dir_open((uint8_t*)".");
  }

  else {
    if(file_open(filename) == -1){return -1;}
    current_pcb->fd_array[index].file_operations_pointer = file_table;
    current_pcb->fd_array[index].inode = file_dentry.inode_num;
    current_pcb->fd_array[index].file_position = 0;
    current_pcb->fd_array[index].flags = 1;
  }
  return index;

}
// system_close():
// description: close the file/rtc/dir/terminal
// input : none
// output : 0 if success
// side effect : none

int32_t system_close (int32_t fd){
  if(current_pcb->fd_array[fd].flags == 0){return -1;}
	if(fd<2||fd>=MAX_FILES_NUMBER){return -1;}
    if(current_pcb->fd_array[fd].file_operations_pointer.close(fd) != 0){return -1;}
    current_pcb->fd_array[fd].flags = 0;
    return 0;
}


// system_getargs():
// description: read the program's argument into a user-level buffer
// input : buf -- pointer to buffer to store the argument
//         nbytes -- number of bytes to copy
// output : 0 if succeed, -1 if fail
// side effect : change content of buf

int32_t system_getargs(uint8_t* buf, int32_t nbytes){
    if(buf == NULL){  //check buf, if buffer is NULL, fail
      return -1;
    }
    strcpy((int8_t*)buf, (int8_t*)current_pcb->argbuf);       //copy argument from argument buffer in pcb
    // printf("%s\n", buf);
    return 0;
}



// system_vidmap():
// description: maps the text-mode video memory into user space at a pre-set virtual address
// input : screen_start -- pointer to start of user video memory
// output : returns 0 if succeed, -1 if fail
// side effect : effects the physical video memory

int32_t system_vidmap(uint8_t** screen_start){
    if( screen_start == NULL || screen_start == (uint8_t**)_4MB){
      return -1;
    }
    //the first 4KB in 256MB
    page_dir[_256MB/_4MB].page_tbl_base_addr_k =  ((uint32_t) userpage_tbl >> 12);
    page_dir[_256MB/_4MB].present_k = 1;
    page_dir[_256MB/_4MB].read_write_k = 1;
    page_dir[_256MB/_4MB].page_size_k = 0;
    page_dir[_256MB/_4MB].user_supervisor_k = 1;

    userpage_tbl[0].page_base_addr = VIDEO_MEM_ADDR >> 12;
    userpage_tbl[0].present = 1;
    userpage_tbl[0].read_write = 1;
    userpage_tbl[0].user_supervisor = 1;
    flush_tlb();
    *screen_start = (uint8_t*)_256MB ;
    current_pcb->vidmap_flag = 1;
    return 0;
}


// system_set_handler():
// description: nothing
// input : none
// output : 0 if success
// side effect : none

int32_t system_set_handler (int32_t signum, void* handler_address){
    return 0;
}

// system_sigreturn():
// description: nothing
// input : none
// output : 0 if success
// side effect : none

int32_t system_sigreturn (void){
    return 0;
}


// fake_terminal_write():
// description: nothing
// input : none
// output : -1 because invalid
// side effect : none
int32_t fake_terminal_write(int32_t fd, const void* buf, int32_t nbytes){
  return -1;
}

// fake_terminal_read():
// description: nothing
// input : none
// output : -1 because invalid
// side effect : none
int32_t fake_terminal_read(int32_t fd, void* buf, int32_t nbytes){
  return -1;
}
