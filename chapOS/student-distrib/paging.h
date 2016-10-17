#ifndef PAGING_H
#define PAGING_H

#define PAGE_DIRECTORY_SIZE 1024

/* This is a Page-Directory Entry (4-KByte Page Table) */
//See page 90 IA32-ref-manual-vol3 for more info
//+31------------------12+11----9+-8-+-7-+-6-+-5-+-4-+-3-+-2-+-1-+-0-+
//| Page-Table Base Addr | Avail | G | PS| 0 | A |PCD|PWT|U/S|R/W| P |
//+----------------------+-------+---+---+---+---+---+---+---+---+---+
typedef struct pde_4k {
  union {
    uint32_t val;
    struct {
      uint32_t present : 1;
      uint32_t read_write : 1;
      uint32_t user_supervisor : 1;
      uint32_t write_through : 1;
      uint32_t cache_disabled : 1;
      uint32_t accessed : 1;
      uint32_t reserved : 1;    //set to 0
      uint32_t page_size : 1;   //0 indicates 4 KBytes
      uint32_t global_page : 1; //(ignored)
      uint32_t avail : 3;       //Available for system programmer's use
      uint32_t base_addr : 20;
    } __attribute__((packed));
  };
}pde_4k_t;

//This is a Page-Table Entry (4-KByte Page)
typedef struct pte_4k {
  union {
    uint32_t val;
    struct {
      uint32_t present : 1;
      uint32_t read_write : 1;
      uint32_t user_supervisor : 1;
      uint32_t write_through : 1;
      uint32_t cache_disabled : 1;
      uint32_t accessed : 1;
      uint32_t dirty : 1;
      uint32_t pt_attrib_indx : 1;
      uint32_t global_page : 1;
      uint32_t avail : 3;       //Available for system programmer's use
      uint32_t base_addr : 20;
    } __attribute__((packed));
  };
}pte_4k_t;

//This is a Page-Directory Entry (4-MByte Page)
typedef struct pde_4m {
  union {
    uint32_t val;
    struct {
      uint32_t present : 1;
      uint32_t read_write : 1;
      uint32_t user_supervisor : 1;
      uint32_t write_through : 1;
      uint32_t cache_disabled : 1;
      uint32_t accessed : 1;
      uint32_t dirty : 1;
      uint32_t page_size : 1;   //1 indicates 4 MBytes
      uint32_t global_page : 1;
      uint32_t avail : 3;       //Available for system programmer's use
      uint32_t pt_attrib_indx : 1;
      uint32_t reserved : 9;
      uint32_t base_addr : 10;
    } __attribute__((packed));
  };
}pde_4m_t;

typedef struct pdt{
  pde_4m_t pde_4m;
  pde_4k_t pde_4k;
}pdt_t;

//Initialize paging parameters and registers
void initialize_paging(void);

#endif