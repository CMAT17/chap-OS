#ifndef PAGING_H
#define PAGING_H

#include "type.h"
#include "x86_desc.h"

#define PAGE_DIRECTORY_SIZE 1024
#define PAGE_TABLE_SIZE 1024
#define PAGE_ALIGN 4096

#ifndef VIDEO
#define VIDEO 0xB8000
#endif

//Initialize paging parameters and registers
void initialize_paging(void);


#endif