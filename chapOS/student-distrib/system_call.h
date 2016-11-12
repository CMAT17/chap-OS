#ifndef _SYSTEM_CALL_H
#define _SYSTEM_CALL_H

/* Adding all the possible includes that we might need */
#include "types.h"
#include "paging.h"
#include "keyboard.h"
#include "lib.h"
#include "rtc.h"

//add?
#include "file_sys_module.h"

//define magic numbers

#define MAX_ARG_SIZE	100
#define MAX_NAME_SIZE	10
#define NULL_CHAR		'\0'
#define MAX_OPEN_FILE	8

#define MAX_FILE_SIZE 32

//extern
int32_t halt(uint8_t status);
int32_t execute(const uint8_t* command);
int32_t read(int32_t fd, void* buf, int32_t nbytes);
int32_t write(int32_t fd, const void* buf, int32_t nbytes);
int32_t open(const uint8_t* filename);
int32_t close(int32_t fd);
int32_t getargs(uint8_t* buf, int32_t nbytes);
int32_t vidmap(uint8_t** screen_start);

/* Following two are for extra credit and later*/
int32_t set_handler (int32_t signum, void* handler_address);
int32_t sigreturn (void);

/*
typedef struct{
    int32_t (*open)(const uint8_t * filename);
    int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
    int32_t (*close)(int32_t fd);
} file_ops_jmp_tb_t;
*/

/*
typedef struct{
    file_ops_jmp_tb_t* fops_jmp_tb_ptr;
    int32_t inode;
    int32_t file_pos;
    int32_t flags;
} pcb_entry_t;
*/


#endif /* _SYSTEM_CALL_H */
