#include "filesyshelper.h"
#include "lib.h"
#include "systemcall_function.h"


#define BLOCK_SIZE 4096
#define DENTRY_SIZE 64
#define FILE_NAME_LENGTH 32

int32_t last_dentry_read;
int32_t last_position_read;
dentry_t file_dentry;
dentry_t dir_dentry;
static uint32_t* file_sys_addr;

// init_file_sys():
// description:initial all the parameter
// input:None
// output:None
// side effect: file_name and sys_addr will be initialized

void init_file_sys(uint32_t* filesys_addr) {
    file_dentry.file_name[FILE_NAME_LENGTH] = '\0';
    dir_dentry.file_name[FILE_NAME_LENGTH] = '\0';
    file_sys_addr = filesys_addr;
}


// read_dentry_by_name():
// description: find the correct file and load its name, type, inode into dentry
// input: fname :the name of the file needed to be find
//        dentry: the place where to store  the file_name, file_type and file inode_num
// output: 0 if success
//         -1 if fail
// side effect:  the dentry will be filled

int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry) {
    uint8_t* entry_start = (uint8_t*) file_sys_addr;
    uint32_t dentry_num = *file_sys_addr;
    uint32_t size, sizef;
    uint8_t* name_pointer;
    entry_start += DENTRY_SIZE;
    // loop through evey dentry
    for (name_pointer = entry_start; name_pointer < entry_start + dentry_num * DENTRY_SIZE; name_pointer += DENTRY_SIZE){
      size = strlen((int8_t*)fname);
      sizef = (FILE_NAME_LENGTH > strlen((int8_t*)name_pointer))? strlen((int8_t*)name_pointer): FILE_NAME_LENGTH;
      // continue if the length doesn't match
      if (size != sizef)
          continue;
      // match!
      if (!strncmp((int8_t*)name_pointer, (int8_t*)fname, size)) {
        memcpy(dentry->file_name, (int8_t*)name_pointer, FILE_NAME_LENGTH);
        dentry->file_type = (uint32_t) *(name_pointer + FILE_NAME_LENGTH);
        dentry->inode_num = (uint32_t) *(name_pointer + FILE_NAME_LENGTH + sizeof(int32_t));
        return 0;
      }
    }
    return -1;
}

// read_dentry_by_index():
// description: find the dentry by index and olad its name, type, inode into dentry
// input : index : the file need to find
//         dentry : the place where to store the file _name, file_type and file inode_node
// output : 0 if success
//           -1 if fail
// side effect: the dentry will be filled

int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry) {
    uint32_t dentry_num = *file_sys_addr;
    uint8_t* dentry_ptr = (uint8_t*) file_sys_addr;
    if (index >= dentry_num) {
        return -1;
    }
    // calculate dentry position
    dentry_ptr += (index + 1) * DENTRY_SIZE;
    memcpy(dentry->file_name, (int8_t*)dentry_ptr, FILE_NAME_LENGTH);
    dentry->file_type = (uint32_t) *(dentry_ptr + FILE_NAME_LENGTH);
    dentry->inode_num = (uint32_t) *(dentry_ptr + FILE_NAME_LENGTH + sizeof(int32_t));
    return 0;
}

// read_data():
// descroption: find the position inode block with offset, read date with length into buf
// input : inode : the inode index store the file data
//         offset : the offset from the beginning of the inode
//         buf : where the data should be stored
//         length : the length of the data we should read
// output : length if success
//          -1 if fail
// side effect :none
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) {
    uint32_t inode_num;
    uint32_t* inode_ptr;
    uint32_t inode_length;
    uint8_t* data_block_ptr, *data_block_base;
    uint32_t data_block;

    inode_num = *(file_sys_addr + 1);
    inode_ptr = file_sys_addr + (inode + 1) * BLOCK_SIZE / (sizeof(uint32_t));
    inode_length = *inode_ptr;

    if (inode >= inode_num) {
        return -1;
    }

    if (offset >= inode_length) {
        return -1;
    }
    // modified length to fit with one data block
    if ((BLOCK_SIZE - offset % BLOCK_SIZE) < length) {
        length = BLOCK_SIZE - offset % BLOCK_SIZE;
    }
    // read till the end of the file
    if (offset + length > inode_length) {
        length -= offset + length - inode_length;
    }
    // get the dota block number desired and pointer to it in the file system
    data_block = *(inode_ptr + 1 + offset / BLOCK_SIZE);
    data_block_base = (uint8_t*) file_sys_addr;
    data_block_base += ((inode_num + 1) * BLOCK_SIZE/(sizeof(uint8_t)));
    data_block_ptr = data_block_base + (BLOCK_SIZE * data_block + offset % BLOCK_SIZE);
    // reading and record position
    memcpy((int8_t*)buf, (int8_t*)data_block_ptr, length);
    last_position_read += length;
    return length;
}

// file_open():
// description: open the file
// input : fname : the name of the file we need to open
// output : 0 if success
//          -1 if fail
// side effect : the file will be open and the last_position_read will be membered
int32_t file_open (const uint8_t* fname) {
    last_position_read = 0;
    return read_dentry_by_name(fname, &file_dentry);
}

// file_close():
// description: close the file
// input : fd : the file descriptor of the file we need to open
// output : 0 if success
// side effect : none
int32_t file_close (int32_t fd) {
    return 0;
}


// file_write():
// description: write nbytes from buf into the file
// input : fd : the file descriptor of the file we need to write
//         buf : where the data stored
//         nbytes: the length of data need to write
// output : 0 if success
// side effect : none
int32_t file_write (int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}

// file_read():
// description: read nbytes from file into the buf
// input : fd : the file descriptor of the file we need to write
//         buf : where the data stored
//         nbytes: the length of data need to read
// output : 0 if success
// side effect : none
int32_t file_read (int32_t fd, void* buf, int32_t nbytes) {
    int32_t count, cnt;
    count = 0;
    fd_t file_desc = current_pcb->fd_array[fd];
    // Read until error reported from read_data
    while (nbytes - count > 0) {
        cnt = read_data(file_desc.inode, file_desc.file_position + count, buf + count, nbytes - count);
        if (cnt != -1) {
            count += cnt;
        } else
            break;
    }
  return count;
}

// dir_open():
// description: open the directory
// input : fname : the name of the directory we need to open
// output : 0 if success
//          -1 if fail
// side effect : the file will be open and the last_dentry_read will be membered
int32_t dir_open (const uint8_t* filename) {
    last_dentry_read = 0;
    return read_dentry_by_name(filename, &dir_dentry);
}

// dir_close():
// description: close the directory
// input : none
// output : 0 if success
// side effect : none
int32_t dir_close (int32_t fd){
  last_dentry_read = 0;
  return 0;
}

// dir_write():
// description: write nbytes from buf into the directory
// input : none
// output : 0 if success
// side effect : none
int32_t dir_write (int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}

// dir_read():
// description: read one dir and put its all file info to screen
// input : fd : the file descriptor of the file we need to write
//         buf : where the data stored
//         nbytes: the length of data need to read
// output : 0 if success
// side effect : none
int32_t dir_read (int32_t fd, void* buf, int32_t nbytes) {
  int32_t* inode_ptr;
  int32_t ret;

  uint32_t dentry_num = *file_sys_addr;
  uint8_t* dentry_ptr = (uint8_t*) file_sys_addr;
  dentry_ptr += (last_dentry_read + 1) * DENTRY_SIZE;
  if (last_dentry_read >= dentry_num) {
      return 0;
  }
  // calculate the inode_ptr
  inode_ptr = (int32_t*)file_sys_addr + ((dir_dentry.inode_num + 1) * BLOCK_SIZE / (sizeof(uint32_t)));
  // Reading
  strcpy(buf, (int8_t*)dir_dentry.file_name);
  ret = strlen((int8_t*)dir_dentry.file_name);
  last_dentry_read ++;
  read_dentry_by_index (last_dentry_read, &dir_dentry);
  return ret;
}
