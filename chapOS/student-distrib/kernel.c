/* kernel.c - the C part of the kernel
 * vim:ts=4 noexpandtab
 */

/* Points to reclaim: RTC interrupts (MP3.1)
 * 					  Keyboard Interrupts (MP3.1)
 * 					  
 *
 */


#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "rtc.h"
#include "keyboard.h"
#include "paging.h"
#include "types.h"
#include "file_sys_module.h"
#include "system_call.h"

/* Macros. */
/* Check if the bit BIT in FLAGS is set. */
#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))

/* Check if MAGIC is valid and print the Multiboot information structure
   pointed by ADDR. */
void
entry (unsigned long magic, unsigned long addr)
{
	multiboot_info_t *mbi;
	module_t* file_sys_module;
	uint8_t * null_filename = NULL;

	/* Clear the screen. */
	clear();

	/* Am I booted by a Multiboot-compliant boot loader? */
	if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
	{
		printf ("Invalid magic number: 0x%#x\n", (unsigned) magic);
		return;
	}

	/* Set MBI to the address of the Multiboot information structure. */
	mbi = (multiboot_info_t *) addr;

	/* Print out the flags. */
	printf ("flags = 0x%#x\n", (unsigned) mbi->flags);

	/* Are mem_* valid? */
	if (CHECK_FLAG (mbi->flags, 0))
		printf ("mem_lower = %uKB, mem_upper = %uKB\n",
				(unsigned) mbi->mem_lower, (unsigned) mbi->mem_upper);

	/* Is boot_device valid? */
	if (CHECK_FLAG (mbi->flags, 1))
		printf ("boot_device = 0x%#x\n", (unsigned) mbi->boot_device);

	/* Is the command line passed? */
	if (CHECK_FLAG (mbi->flags, 2))
		printf ("cmdline = %s\n", (char *) mbi->cmdline);

	if (CHECK_FLAG (mbi->flags, 3)) {
		int mod_count = 0;
		int i;
		module_t* mod = (module_t*)mbi->mods_addr;
		while(mod_count < mbi->mods_count) {
			printf("Module %d loaded at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_start);
			printf("Module %d ends at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_end);
			printf("First few bytes of module:\n");
			for(i = 0; i<16; i++) {
				printf("0x%x ", *((char*)(mod->mod_start+i)));
			}
			printf("\n");
			mod_count++;
			mod++;
		}
	}
	/* Bits 4 and 5 are mutually exclusive! */
	if (CHECK_FLAG (mbi->flags, 4) && CHECK_FLAG (mbi->flags, 5))
	{
		printf ("Both bits 4 and 5 are set.\n");
		return;
	}

	/* Is the section header table of ELF valid? */
	if (CHECK_FLAG (mbi->flags, 5))
	{
		elf_section_header_table_t *elf_sec = &(mbi->elf_sec);

		printf ("elf_sec: num = %u, size = 0x%#x,"
				" addr = 0x%#x, shndx = 0x%#x\n",
				(unsigned) elf_sec->num, (unsigned) elf_sec->size,
				(unsigned) elf_sec->addr, (unsigned) elf_sec->shndx);
	}

	/* Are mmap_* valid? */
	if (CHECK_FLAG (mbi->flags, 6))
	{
		memory_map_t *mmap;

		printf ("mmap_addr = 0x%#x, mmap_length = 0x%x\n",
				(unsigned) mbi->mmap_addr, (unsigned) mbi->mmap_length);
		for (mmap = (memory_map_t *) mbi->mmap_addr;
				(unsigned long) mmap < mbi->mmap_addr + mbi->mmap_length;
				mmap = (memory_map_t *) ((unsigned long) mmap
					+ mmap->size + sizeof (mmap->size)))
			printf (" size = 0x%x,     base_addr = 0x%#x%#x\n"
					"     type = 0x%x,  length    = 0x%#x%#x\n",
					(unsigned) mmap->size,
					(unsigned) mmap->base_addr_high,
					(unsigned) mmap->base_addr_low,
					(unsigned) mmap->type,
					(unsigned) mmap->length_high,
					(unsigned) mmap->length_low);
	}

	/* Construct an LDT entry in the GDT */
	{
		seg_desc_t the_ldt_desc;
		the_ldt_desc.granularity    = 0;
		the_ldt_desc.opsize         = 1;
		the_ldt_desc.reserved       = 0;
		the_ldt_desc.avail          = 0;
		the_ldt_desc.present        = 1;
		the_ldt_desc.dpl            = 0x0;
		the_ldt_desc.sys            = 0;
		the_ldt_desc.type           = 0x2;

		SET_LDT_PARAMS(the_ldt_desc, &ldt, ldt_size);
		ldt_desc_ptr = the_ldt_desc;
		lldt(KERNEL_LDT);
	}

	/* Construct a TSS entry in the GDT */
	{
		seg_desc_t the_tss_desc;
		the_tss_desc.granularity    = 0;
		the_tss_desc.opsize         = 0;
		the_tss_desc.reserved       = 0;
		the_tss_desc.avail          = 0;
		the_tss_desc.seg_lim_19_16  = TSS_SIZE & 0x000F0000;
		the_tss_desc.present        = 1;
		the_tss_desc.dpl            = 0x0;
		the_tss_desc.sys            = 0;
		the_tss_desc.type           = 0x9;
		the_tss_desc.seg_lim_15_00  = TSS_SIZE & 0x0000FFFF;

		SET_TSS_PARAMS(the_tss_desc, &tss, tss_size);

		tss_desc_ptr = the_tss_desc;

		tss.ldt_segment_selector = KERNEL_LDT;
		tss.ss0 = KERNEL_DS;
		tss.esp0 = 0x800000;
		ltr(KERNEL_TSS);
	}
	//obtain the filesystem structure
	file_sys_module = (module_t *)mbi->mods_addr;
	file_sys_init(file_sys_module);
	/* Init the PIC */
	i8259_init();

	/* Initialize devices, memory, filesystem, enable device interrupts on the
	 * PIC, any other initialization stuff... */
	// Init the RTC
	rtc_init();

	//initialize the keyboard
	open_keyboard(null_filename);
	/* Enable interrupts */
	/* Do not enable the following until after you have set up your
	 * IDT correctly otherwise QEMU will triple fault and simple close
	 * without showing you any output */
	printf("Enabling Interrupts\n");
	sti();
	
  initialize_paging();
  
//-----------------------------Sandwich testing--------------------------------

  /*
  long testtt=0;
  for(testtt=0;testtt<100000000;testtt++){
    
  }*/
  char testChar[50];
  char testbuf[33];
  uint8_t testfcontents[10000];
  int32_t fd = 0;
  int32_t unused_fd = -1;
  int32_t nbytes = 0;
  int32_t i;
  dentry_t dentry;
  //char writeArray[50] = "This is written message";
  printf("Start keyboard read test. Please type to fill in the buffer\n");
  //write_keyboard(writeArray,50);
  keyboard_read(unused_fd,testChar,50);
  printf("The message in the buffer is \"");
  printf("%s\"\n",testChar);
  /*
  printf("Start RTC read test\n");
  while(1){
    rtc_read();
    printf("1");
  }
  */

  printf("Start Directory Read Test\n");
  for(i = 0; i<20; i++){
  	dir_read(fd, testbuf, nbytes);
  	printf("%s\n",testbuf);
  }
  printf("Start File Read Test \n");
  read_dentry_by_name((uint8_t*)"shell", &dentry);
  printf("shell\n");
  uint32_t len = get_file_size(dentry.inode_num);
  printf("%d\n", len);
  printf("%d\n",read_data(dentry.inode_num, 0, testfcontents, len));
  printf("%s\n", testfcontents);
  execute((uint8_t*)"shell");
//---------------------------End Sandwich testing------------------------------
  //uint8_t testTXT[100] = "     shell       This is Args";
  //execute(testTXT);
  //printf("Done test CMD\n");
  //testing page faults
  /*
  int* test;
  test = (int*)(0x200000);
  int test2 = test[0];
  
  printf("%i", test2);
  */
  //testing div by 0
  //int a = 3/0;
  /*
  int* dummy = 0x08048000;
  new4MB_page();
  *dummy = 555;
  printf("%d\n",*dummy);
  new4MB_page();
  *dummy = 666;
  printf("%d\n",*dummy);
  new4MB_page();
  printf("%d\n",*dummy);
  rm4MB_page();
  printf("%d\n",*dummy);
  rm4MB_page();
  printf("%d\n",*dummy);*/

  //testing GP
  //int *a = NULL;
  //int b = *a;
	/* Execute the first program (`shell') ... */

	/* Spin (nicely, so we don't chew up cycles) */

	asm volatile(".1: hlt; jmp .1;");
}

