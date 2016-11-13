#ifndef _FILE_SYS_H_
#define _FILE_SYS_H_

#include "types.h"
#include "multiboot.h"

#define NUM_RESERVED_DENTRY 24
#define NUM_RESERVED_BOOT   52
#define FILE_NAME_SIZE      32
#define MAX_DENTRY_NUM      63
#define MAX_DATA_BLOCK_NUM  1023
#define DATA_BLOCK_SIZE     4096

typedef struct dentry{
    uint8_t file_name[FILE_NAME_SIZE];
    uint32_t file_type;
    uint32_t inode_num;
    uint8_t reserved[NUM_RESERVED_DENTRY];    
} dentry_t;

typedef struct boot_block{
    uint32_t num_dentry;
    uint32_t num_inodes;
    uint32_t num_data_blocks;
    uint8_t reserved[NUM_RESERVED_BOOT];
    dentry_t dir_entries[MAX_DENTRY_NUM];
} boot_block_t;

typedef struct inode{
    uint32_t length;
    uint32_t data_block[MAX_DATA_BLOCK_NUM];
} inode_t;

void file_sys_init(module_t* file_sys_module);
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
int32_t file_open(const uint8_t* file_name);
int32_t file_sys_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t file_sys_close(int32_t fd);
int32_t file_sys_open(const uint8_t * filename);

int32_t file_open(const uint8_t * filename);
int32_t file_close(int32_t fd);
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t file_read(int32_t fd, void* buf, int32_t nbytes);

int32_t dir_open(const uint8_t * filename);
int32_t dir_close(int32_t fd);
int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes);


#endif
