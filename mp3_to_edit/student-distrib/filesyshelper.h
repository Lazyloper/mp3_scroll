#if !defined(FILESYSHELPER_H)
#define FILESYSHELPER_H

#include "lib.h"

typedef struct dentry {
    uint8_t file_name[33];   //srt 33 becasue the name has at most 32 char
    uint32_t file_type;
    uint32_t inode_num;
} dentry_t;



extern void init_file_sys(uint32_t* filesys_addr);

extern int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);

extern int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);

extern int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

extern int32_t file_open (const uint8_t* fname);

extern int32_t file_close (int32_t fd);

extern int32_t file_write (int32_t fd, const void* buf, int32_t nbytes);

extern int32_t file_read (int32_t fd, void* buf, int32_t nbytes);

extern int32_t dir_open (const uint8_t* filename);

extern int32_t dir_close (int32_t fd);

extern int32_t dir_write (int32_t fd,const void* buf, int32_t nbytes);

extern int32_t dir_read (int32_t fd, void* buf, int32_t nbytes);

extern int32_t last_dentry_read;

extern int32_t last_position_read;

extern dentry_t file_dentry;

extern dentry_t dir_dentry;

#endif
