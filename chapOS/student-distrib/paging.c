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

uint32_t user_page_table[PAGE_TABLE_SIZE] __attribute__((aligned(PAGE_ALIGN)));

uint8_t num_process;

int32_t new_userVID_page(void* vir_addr){
  page_dir[(uint32_t)vir_addr>>22] = ((uint32_t)user_page_table) | 7;
  user_page_table[((uint32_t)vir_addr>>12)&MASK_10_BITS] = VIDEO | 7;//0x8400007;//0x8400007 | 7;
  return 0;
}

//initialize_paging function
//This function initialize the page directory and page table.
//The first 4MB will be using 4KB paging with the first page directory. 
//Input: none
//Return: none
void initialize_paging(void){
  int i;  //Iterator
  
  //Reset the number of process to be 0
  num_process = 0;
  
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
  
  //Set control registers
  paging_setCR();
  return;
}

//new4MB_page function
//This function will create a new 4MB page for a new process
//It will map to phys mem at 8MB, 12MB, 16MB, ...
//Input: none
//Output: none
int32_t new4MB_page(void){
  //If there is no program running yet, create a new 4MB page at 8MB phy mem
  if(num_process == 0){
    page_dir[PDE_USER_PROG] = PDE_8MB_PHY|PD_SET_4MB|PD_ENABLE_ENTRY;
    num_process = 1;
  }
  //If there is one program running, create a new 4MB page at 12MB phy mem
  else if(num_process == 1){
    page_dir[PDE_USER_PROG] = PDE_12MB_PHY|PD_SET_4MB|PD_ENABLE_ENTRY;
    num_process = 2;
  }
  //Else do nothing, return -1 (error)
  else{
    return -1;
  }
  //Flush TLB
  paging_setCR();
  return 0;
}

//rm4MB_page function
//This function will destroy one 4MB page for current process
//It will map to phys mem at 8MB, 12MB, 16MB, ...
//Input: none
//Output: none
int32_t rm4MB_page(void){
  //If the program is running at 12MB, then change to 8MB
  if(num_process == 2){
    page_dir[PDE_USER_PROG] = PDE_8MB_PHY|PD_SET_4MB|PD_ENABLE_ENTRY;
    num_process = 1;
  }
  //If the program is running at 8MB, then disable the program page
  else if(num_process == 1){
    page_dir[PDE_USER_PROG] = 0;
    num_process = 0;
  }
  //else do nothing and return -1 (error)
  else{
    return -1;
  }
  //Flush CR3
  paging_setCR();
  return 0;
}

//paging_setCR function
//This function sets control registers to enable aging feature.
//Input: none
//Return: none
void paging_setCR(void){
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
}

