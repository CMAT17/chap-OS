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
#define ASCII_CHAR_SIZE 8
#define ASCII_DELETE    0x7F
#define ASCII_E         0x45
#define ASCII_L         0x4C
#define ASCII_F         0x46
#define ASCII_ELF       0x7F454C46

#define MAX_ARG_SIZE	    100
#define MAX_NAME_SIZE	    10
#define NULL_CHAR		    '\0'
#define NUM_OPS	            4
#define PCB_MASK            0xFFFFE000
#define MAX_PROCESSES       6
#define PAGE_8MB            0x800000  
#define STACK_8KB           0x2000
#define PROG_IMAGE_VADDR    0x08048000
#define ENTRY_PTW_START     24      
#define MAX_FILES           8
#define USER_IMG_START      0x8000000
#define USER_IMG_END        0x8400000
//Flag types
#define FLAG_ACTIVE		0x00000001
#define FLAG_INACTIVE	0x00000000

//File Types user level
#define FILE_POS   		0x00000000
#define FILE_TYPE_DIR	0x00000001
#define FILE_TYPE_FILE	0x00000002
#define FILE_TYPE_RTC	0x00000000

#define FDS_STDIN_IDX	0
#define FDS_STDOUT_IDX	1

#define FD_STDIN        0
#define FD_STDOUT       1

//extern
int32_t halt(uint8_t status);
int32_t execute(const uint8_t* command);
int32_t read(int32_t fd, void* buf, int32_t nbytes);
int32_t write(int32_t fd, const void* buf, int32_t nbytes);
int32_t open(const uint8_t* filename);
int32_t close(int32_t fd);
int32_t getargs(uint8_t* buf, int32_t nbytes);
int32_t vidmap(uint8_t** screen_start);

int32_t do_nothing(); 
int32_t gen_new_proc_id(void);

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
