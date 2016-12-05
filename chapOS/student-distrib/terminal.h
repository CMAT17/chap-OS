#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#include "types.h"

#define NUM_TERM        3
#define KEY_BUF_SIZE    128
#define TERM_0          0
#define TERM_1          1
#define TERM_2          2
#define NOT_ACTIVE      0
#define ACTIVE          1
#define IS_IN_USE       1
#define NOT_IN_USE      0

typedef struct terminal_struct{
    uint8_t buffer_index;
    uint8_t term_id;
    uint8_t cur_proc_num;
    uint8_t active_flag;
    uint8_t* term_vid_mem;
    uint8_t x;
    uint8_t y;
    //uint8_t return_flag;
    uint8_t is_in_use_flag; //debugging multi terminal uses, checks which terminal is currently being run
}term_t; 

void init_terminals();

int32_t terminal_restore(uint8_t terminal_id);

int32_t terminal_save(uint8_t terminal_id);

int32_t terminal_switch_term(uint8_t target_terminal_id);

int32_t terminal_launch(uint8_t target_terminal_id);

int32_t terminal_change(uint8_t target_terminal_id);

int32_t terminal_LoS(uint8_t target_terminal_id);



#endif
