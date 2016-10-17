#ifndef PAGING_H
#define PAGING_H

#include "types.h"
#include "x86_desc.h"

//define all the magic numbers
#define PAGE_DIRECTORY_SIZE 1024
#define PAGE_TABLE_SIZE 1024
#define PAGE_ALIGN 4096
#define HI_PTE_MASK 0xFFFFF000
#define PD_ENABLE_ENTRY 0x00000003
#define NOT_PRESENT 2
#define INIT_4MB_KERNEL 0x400083

//VIdo memory location to be map to page table (first page directory)
#ifndef VIDEO
#define VIDEO 0xB8000
#endif
#define HI_VIDEO 0xB8

//Initialize paging parameters and registers
void initialize_paging(void);


#endif
