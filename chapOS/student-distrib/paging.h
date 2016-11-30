#ifndef PAGING_H
#define PAGING_H

#include "types.h"
#include "x86_desc.h"

//define all the magic numbers
#define PAGE_DIRECTORY_SIZE   1024
#define PAGE_TABLE_SIZE       1024
#define PAGE_ALIGN            4096
#define HI_PTE_MASK           0xFFFFF000
#define PD_ENABLE_ENTRY       0x00000003
#define MAKE_ENTRY_PRESENT    0x00000001
#define ENABLE_RW             0x00000002
#define PD_SET_4MB            0x00000080
#define ALLOW_USER_LEVEL      0x00000004
#define INIT_4MB_KERNEL       0x400083
#define PDE_USER_PROG         0x20
#define PDE_8MB_PHY           0x00800000
#define PDE_12MB_PHY          0x00C00000
#define MASK_10_BITS          0x3ff
#define ENABLE_USER_ENTRY     7

//Video memory location to be map to page table (first page directory)
#ifndef VIDEO
#define VIDEO 0xB8000
#endif
#define HI_VIDEO 0xB8

//Initialize paging parameters and registers
void initialize_paging(void);

//Set controll registers for paging
void paging_setCR(void);

//Enable new 4MB page for new program
int32_t new4MB_page(void);

//Tear down 4MB page for the program
int32_t rm4MB_page(void);

//Create user level video mapping page
int32_t new_userVID_page(void* vir_addr);

#endif
