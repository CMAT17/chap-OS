#ifndef _FILE_SYS_H_
#define _FILE_SYS_H_

#include "types.h"
#include "multiboot.h"
#include "system_call.h"

#define NUM_RESERVED_DENTRY 24
#define NUM_RESERVED_BOOT   52
#define FILE_NAME_SIZE      32
#define MAX_DENTRY_NUM      63
#define MAX_DATA_BLOCK_NUM  1023
#define DATA_BLOCK_SIZE     4096
#define MIN_READ_ELF_SIZE   4

#define MAX_ARG_SIZE    100
#define MAX_NAME_SIZE   10
#define NULL_CHAR       '\0'
#define MAX_OPEN_FILE   8
#define MIN_OPEN_FILE   2
#define MAX_FILE_SIZE  32

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

typedef struct file_ops_jmp_tb{
    int32_t (*open)(const uint8_t*);
    int32_t (*read)(int32_t, void* buf, int32_t);
    int32_t (*write)(int32_t, const void* buf, int32_t);
    int32_t (*close)(int32_t);
} file_ops_jmp_tb_t;

typedef struct file_desc{
    file_ops_jmp_tb_t *fops_jmp_tb_ptr;
    uint32_t inode;
    uint32_t file_pos;
    uint32_t flags;
} file_desc_t;

typedef struct pcb{
    file_desc_t f_descs[MAX_OPEN_FILE];
    uint8_t arg_buff[MAX_ARG_SIZE];
    uint8_t f_names[MAX_OPEN_FILE][FILE_NAME_SIZE];
    uint8_t proc_num;
    uint32_t ksp;
    uint32_t kbp;
    uint8_t parent_proc_num;
    uint32_t parent_ksp;
    uint32_t parent_kbp;
    //int32_t canary;
} pcb_t;

void file_sys_init(module_t* file_sys_module);

//Searches filesystem for given filename and if found copies file information into dentry
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

uint32_t get_file_size(uint32_t inode_num);
pcb_t* get_pcb_ptr();


#endif
