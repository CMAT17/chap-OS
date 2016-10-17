#include "paging.h"

//The page directory
/* This is a Page-Directory Entry (4-KByte Page Table) */
//See page 90 IA32-ref-manual-vol3 for more info
//+31------------------12+11----9+-8-+-7-+-6-+-5-+-4-+-3-+-2-+-1-+-0-+
//| Page-Table Base Addr | Avail | G | PS| 0 | A |PCD|PWT|U/S|R/W| P |
//+----------------------+-------+---+---+---+---+---+---+---+---+---+
uint32_t page_dir[PAGE_DIRECTORY_SIZE] __attribute__((aligned(PAGE_ALIGN)));

//The page table stores its entries
uint32_t page_table[PAGE_TABLE_SIZE] __attribute__((aligned(PAGE_ALIGN)));

//initialize_paging function
//This function initialize the page directory and page table.
//The first 4MB will be using 4KB paging with the first page directory. 
//Input: none
//Return: none
void initialize_paging(void){
  int i;  //Iterator
  
  //Initialize the first page directory to point to the first page table
  page_dir[0] = ((uint32_t)page_table & HI_PTE_MASK)|PD_ENABLE_ENTRY;
  page_table[0] = 0;  //Clear the first page_table entry to be 0
  
  //Initialize page directory entries and page table entries before setting their values
  for(i=1;i<PAGE_DIRECTORY_SIZE;i++){
    //This will clear all bits as shown above, but wants to turn on R/W bit
    page_dir[i] = NOT_PRESENT;
    page_table[i] = page_table[i-1]+PAGE_ALIGN;
  }
  
  //Set the R/W of the page table to be 1
  for(i=0;i<PAGE_DIRECTORY_SIZE;i++){
    page_table[i] = page_table[i] | NOT_PRESENT; //Enable R/W for page table
  }
  
  page_table[HI_VIDEO] |= PD_ENABLE_ENTRY;//1;     //Set the video memory page table to be present

  
  //Set the second entry of the page directory to be 4MB for the kernel
  //10000011 enable RW, present, and set it to be 4MB
  page_dir[1] |= INIT_4MB_KERNEL; //This Starts at 4MB (kernel)
  
  //Set cr3(PDBR) to point to PD, enable paging, Page size extension
  asm("movl %0, %%eax          \n"
      "movl %%eax, %%cr3       \n"
      "movl %%cr4, %%eax       \n"
      "orl  $0x00000010, %%eax \n"
      "movl %%eax, %%cr4       \n"
      "movl %%cr0, %%eax       \n"
      "orl  $0x80000000, %%eax \n"
      "movl %%eax, %%cr0       \n"
      :                        // no output
      :"r"(page_dir)           // page_dir as the input
      :"%eax","cc"             // clobbered register
  );
  return;
}



