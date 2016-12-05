/* Adding all the possible includes that we might need */
#include "system_call.h"
#include "file_sys_module.h"
#include "paging.h"
#include "x86_desc.h"
#include "lib.h"
#include "keyboard.h"
#include "i8259.h"

//Array to keep track of which processes are active
int proc_id_flags[MAX_PROCESSES] = {0,0,0,0,0,0};
//File operation tables (rtc, file, dir, stdin, stdout, no_file)
file_ops_jmp_tb_t rtc__ops_tbl = { rtc_open, rtc_read, rtc_write, rtc_close};
file_ops_jmp_tb_t file_ops_tbl = { file_open, file_read, file_write, file_close};
file_ops_jmp_tb_t dir_ops_tbl = { dir_open, dir_read, dir_write, dir_close };
file_ops_jmp_tb_t stdin_ops_tbl = { open_keyboard, keyboard_read, do_nothing, close_keyboard };
file_ops_jmp_tb_t stdout_ops_tbl = { open_keyboard, do_nothing, keyboard_write, close_keyboard};
file_ops_jmp_tb_t no_file_ops = {do_nothing, do_nothing, do_nothing, do_nothing};

//Keep track the number of active processes
static uint32_t active_proc_num;

//static pcb_t* cur_pcb_ptr;

//Sandwich added
extern uint32_t page_dir[PAGE_DIRECTORY_SIZE] __attribute__((aligned(PAGE_ALIGN)));

/* Notes for ourselves:
	Restore
	-pcb
	-esp, ebp
	-paging structures
	-tss, esp0
*/
/* int32_t halt(uint8_t status)
 * Will terminate the process and close all the files
 * INPUT: status  
 * RETURN VALUE: 0, but will never be reach do to iret
 */
int32_t 
halt(uint8_t status) {
  cli();
  int32_t i;
  uint32_t prev_active_proc_num = active_proc_num;
  pcb_t * cur_PCB;

  //cur_PCB = (pcb_t *)(PAGE_8MB-STACK_8KB*(active_proc_num+1));
  cur_PCB = get_pcb_ptr();
  //clear process for use
  proc_id_flags[active_proc_num] = FLAG_INACTIVE;

  //close all files
  for(i = 0; i<MAX_FILES; i++){
    if(cur_PCB->f_descs[i].flags == FLAG_ACTIVE)
      close(i);
    cur_PCB->f_descs[i].fops_jmp_tb_ptr = &no_file_ops;
  }

  
  active_proc_num = cur_PCB->parent_proc_num;
  
  //closing involves closing thet keyboard IRQ line, so it must be reenabled

  //enable_irq(KEYBOARD_IRQ);
  mapUserImgPage(cur_PCB->parent_proc_num);
  //rm4MB_page();
  //printf("%d\n", tss.esp0);
  //tss.esp0 = PAGE_8MB-STACK_8KB*(active_proc_num)-4;
  
  //if(active_proc_num==0)
    /*asm volatile(
                "movl %%esp, %0   \n"
                : "=r"(tss.esp0)
    );*/
  //else
    tss.esp0 = PAGE_8MB-STACK_8KB*(active_proc_num)-4;
  
  //current working
  //tss.esp0 = cur_PCB->ksp;
  
  //printf("%d\n", tss.esp0);
  //sti();
  if(prev_active_proc_num==0){
    /*asm volatile(
                  "cli                        \n"
                  "movl   %0, %%eax           \n"
                  "movl   %1, %%ebp           \n"
                  "movl   %2, %%esp           \n"
                  :
                  : "r"((uint32_t)status), "r"(cur_PCB->kbp), "r"(cur_PCB->ksp)
                  :"cc"
              );*/
    execute((uint8_t*)"shell");
  }
  else
    asm volatile(
                    "cli                        \n"
                    "xorl   %%eax,%%eax         \n"
                    "movl   %0, %%eax            \n"
                    "andl   $0xFF, %%eax        \n"
                    //"movl   $1, %%eax            \n"
                    "movl   %1, %%ebp           \n"
                    "movl   %2, %%esp           \n"
                    "cmpl   $0x00000001,%%eax   \n"
                    "jne    RETURN_VAL_SET      \n"
                    "movl   $256,%%eax          \n"
                    "RETURN_VAL_SET:            \n"
                    "jmp    RET_FROM_IRET       \n"
                    :
                    : "r"((uint32_t)status), "r"(cur_PCB->kbp), "r"(cur_PCB->ksp)
                    :"cc"
                );
  //printf("test");
  return 0;
}

/* Notes for ourselves:
 * Execute()
 * 1) Parse command
 * 2) EXE check
 * 3) Reorganize virtual memory
 * 4) File Loader
 * 5) PCB
 ~~ Setup stdin and stdout
 * 6) Context/TSS switch
 * 7) IRET crap 
 */
/* int32_t execute(const uint8_t* command)
 * Parse the command for the command name and argument.
 * Load and execute the new program by setting up a process to handle the new program
 * INPUT: command  
 * RETURN VALUE: Return the status of the function
 */
int32_t 
execute(const uint8_t* command) {
  //Disable interrupts
  cli();
  //parsing the command into filename and argument
 /* uint8_t parsed_fname[MAX_FILE_SIZE];
    uint8_t parsed_arg[]
    int fname_start, fname end;
  */
  int i = 0;
  int name_starting_point = -1;
  int name_ending_point = -1;
  int arg_starting_point = -1;
  int arg_ending_point = -1;
  int8_t file_name_command[MAX_NAME_SIZE];
  int8_t arg_command[MAX_ARG_SIZE];
  uint8_t f_content_buf[MIN_READ_ELF_SIZE];
  dentry_t f_dentry;
  int32_t f_content = 0;
  uint32_t entry_point;
  int32_t new_proc_id;
  //int32_t proc_stat;

  // The command does not exist
  if( command == NULL )
    return -1;

  // Parsing the Program Name
  
  for(i=0;i<MAX_NAME_SIZE;i++)
    file_name_command[i] = NULL_CHAR;
  for(i=0;i<MAX_ARG_SIZE;i++)
    arg_command[i] = NULL_CHAR;
  i = 0;
  //Traverse through the command for the name and argument
  while(command[i] != NULL_CHAR){
    //Look for beginning of the name
    if( name_starting_point == -1){
      if( command[i] == ' ')
        i++;
      else
        name_starting_point = i;
    } 
    //Look for the ending of the name and store it in an array
    else if( name_ending_point == -1){
      if( command[i] != ' '){
        file_name_command[i-name_starting_point] = (int8_t)command[i];
        i++;
      }
      else{
        name_ending_point = i;
        file_name_command[i-name_starting_point] = NULL_CHAR;
      }
    }
    //Look for beginning of the arg
    else if( arg_starting_point == -1){
      if( command[i] == ' ')
        i++;
      else
        arg_starting_point = i;
    } 
    //Look for the ending of the arg and store it in an array
    else if( arg_ending_point == -1){
      if( command[i] != ' '){
        arg_command[i-arg_starting_point] = (int8_t)command[i];
        i++;
      }
      else{
        arg_ending_point = i;
        arg_command[i-arg_starting_point] = NULL_CHAR; 
        printf("See arg Space\n");
        break;
      }
    }
    else
      return -1;
  }

  //Testing of parse
  //printf("The name of command: %s\n", file_name_command );
  //printf("The arg of command: %s\n ", arg_command );

  //Make sure that file exists
  if(read_dentry_by_name((uint8_t *) file_name_command, & f_dentry))
  {
    //printf ("Shits done fucked up\n");
    return -1;
  }

  //obtain the first 4 bytes of the file to check if file is executable
  read_data(f_dentry.inode_num, 0, f_content_buf, MIN_READ_ELF_SIZE);
  
  //Append the file content buffer together to compare with the ELF char
  for(i = 0; i<MIN_READ_ELF_SIZE-1; i++){
    f_content |= f_content_buf[i];
    f_content = f_content<<ASCII_CHAR_SIZE;
  }

  //Append the last byte outside of loop to avoid loss of data
  f_content |= f_content_buf[MIN_READ_ELF_SIZE-1];
  
  if(f_content != ASCII_ELF){
    printf(" \n");
    return -1;
  }
  
  //obtain entry point
  read_data(f_dentry.inode_num, ENTRY_PTW_START,f_content_buf, MIN_READ_ELF_SIZE);
  entry_point = *((uint32_t *) f_content_buf);

  //re-organize virtual memory
  //proc_stat = new4MB_page();
  
  new_proc_id = gen_new_proc_id();
  if(new_proc_id == -1)
  {
    printf("Too many processes\n");
    return -1;
  }
  mapUserImgPage(new_proc_id);

  //Load file into virtual memory
  uint32_t len = get_file_size(f_dentry.inode_num);
  //printf("%d\n", read_data(f_dentry.inode_num, 0, (uint8_t*)PROG_IMAGE_VADDR, len));
  read_data(f_dentry.inode_num, 0, (uint8_t*)PROG_IMAGE_VADDR, len);

  //PCB Stuff
  pcb_t * proc_PCB;
  pcb_t * parent_PCB; 
  
  //new_proc_id = gen_new_proc_id();
  
  if(new_proc_id<0)
  {
    return -1;
  }

  proc_PCB = (pcb_t*)(PAGE_8MB-STACK_8KB*(new_proc_id + 1));
  proc_PCB->proc_num = new_proc_id;
  active_proc_num = new_proc_id;

  //cur_pcb_ptr = proc_PCB;
  
  asm volatile(
              "movl   %%ebp, %0   \n"
              "movl   %%esp, %1   \n"
              : "=r"(proc_PCB->kbp), "=r"(proc_PCB->ksp)
              : //no inputs
              );

  //Set the values for stdin open
  proc_PCB->f_descs[FDS_STDIN_IDX].flags = FLAG_ACTIVE;
  proc_PCB->f_descs[FDS_STDIN_IDX].fops_jmp_tb_ptr = &stdin_ops_tbl;

  //Set the values for stdin open
  proc_PCB->f_descs[FDS_STDOUT_IDX].flags = FLAG_ACTIVE;
  proc_PCB->f_descs[FDS_STDOUT_IDX].fops_jmp_tb_ptr = &stdout_ops_tbl;

  strcpy((int8_t*)(proc_PCB->arg_buff), arg_command);

  //proc_PCB->canary = -32;

  if(new_proc_id==0)
  {
    proc_PCB->parent_proc_num = new_proc_id;
    proc_PCB->parent_ksp = proc_PCB->ksp;
    proc_PCB->parent_kbp = proc_PCB->kbp;
  }
  else
  {
    parent_PCB = (pcb_t *)(PAGE_8MB-STACK_8KB*(proc_PCB->proc_num));
    proc_PCB->parent_proc_num = parent_PCB->proc_num;
    proc_PCB->parent_ksp = parent_PCB->ksp;
    proc_PCB->parent_kbp = parent_PCB->kbp;
  }

  //set tss.ss0 and esp0 to hold kernel data segment and
  tss.ss0 = KERNEL_DS;
  //printf("%d\n",tss.esp0);
  
  /*if(active_proc_num==0)
    asm volatile(
                "movl %%esp, %0   \n"
                : "=r"(tss.esp0)
    );
  else*/
    tss.esp0 = PAGE_8MB-STACK_8KB*(new_proc_id)-4;
  
  //printf("%d\n",tss.esp0);
  //Sandwich added
  
  //tss.cr3 = (uint32_t)page_dir;

  //sti();
/*
  asm volatile(
              "cli                        \n"
              "movw   $0x2B,%%ax          \n"   //User data segment index
              "movw   %%ax, %%ds          \n"   //push to data segment register
              "pushl  $0x2B               \n"   //push user data segment to stack
              "movl   $0x083FFFFC, %%eax  \n"   //push 4-byte aligned bottom of stack
              "pushl  %%eax               \n"   
              "pushfl                     \n"   //push eflags
              "popl   %%eax               \n"
              "orl    $0x4200, %%eax      \n"
              "pushl  %%eax               \n"
              "pushl  $0x23               \n"   //push user code segment
              "movl   %0, %%ebx           \n"   //move entry point into ebx, to be pushed
              "pushl  %%ebx               \n"
              "iret                       \n"
              "RET_FROM_IRET:             \n"
              "leave                      \n"
              "sti                        \n"
              //"ret                        \n"
              : //no outputs
              : "r"(entry_point)
              : "cc", "%ebx", "%eax"
              );*/
  asm volatile("movl %0, %%ebx;jmp _execute_iret\n"::"r"(entry_point):"%ebx");
  return 0;
}

/*
* int32_t open(const uint8_t* filename)
*   Inputs: filename - the file's name to open
*
*   Return Value: return the index of the file which the slot is setup for use
*	Function: Will look for an open slot in the file system to open the file of the pcb
*/
int32_t 
open(const uint8_t* filename) {
  int i; 
  pcb_t * pcb_pointer;
  dentry_t entry; 
  uint32_t dentry_file_type = -1;
  uint32_t check_dentry; 

  pcb_pointer = get_pcb_ptr();

  if(filename == NULL)
  {
    return -1;
  }

  if(filename[0] == NULL_CHAR)
    {
        return -1;
    }

    //Get the dentry and store it in entry and check if it work
  check_dentry = read_dentry_by_name(filename, &entry);

  if( check_dentry == -1)
    return -1;
  else
  {
    dentry_file_type = entry.file_type;
  }

  //Find the index in which the file system is available
  i = MIN_OPEN_FILE;
  while(i) 
  {	
    //Check if maximun number of file is open
    if( i == MAX_OPEN_FILE )
      return -1;

    //Look for an available file slot
    if( pcb_pointer->f_descs[i].flags != FLAG_ACTIVE )
    {
      break;
    }

    i++;
  }

  //Turn that slot active and set up the file position
  pcb_pointer->f_descs[i].flags = FLAG_ACTIVE;
  pcb_pointer->f_descs[i].file_pos = FILE_POS;


  //Set up the jmp table pointer and inode depending on the type of the file
  if( dentry_file_type == FILE_TYPE_DIR)
  {
    if(dir_open(filename) == 0)
    {	
      pcb_pointer->f_descs[i].fops_jmp_tb_ptr = &dir_ops_tbl;
      //inode does not have meaning
      pcb_pointer->f_descs[i].inode = NULL;
      
    }
    else
      return -1;
  }
  else if ( dentry_file_type == FILE_TYPE_FILE)
  {
    if(file_open(filename) == 0)
    {
      pcb_pointer->f_descs[i].fops_jmp_tb_ptr = &file_ops_tbl;
      //Inode only meaningful for file
      pcb_pointer->f_descs[i].inode = entry.inode_num;
            strncpy((int8_t*) pcb_pointer->f_names[i], (int8_t*) filename, MAX_FILE_SIZE);
    }
    else
      return -1;
  }
  else 
  {
    //dentry_file_type == FILE_TYPE_RTC
    if( rtc_open(filename) == 0)
    {
      pcb_pointer->f_descs[i].fops_jmp_tb_ptr = &rtc__ops_tbl;
      //inode does not have meaning
      pcb_pointer->f_descs[i].inode = NULL;
    }
    else
      return -1;
  }

  //Return the index of the file system
  return i;
}

/*
* int32_t read(int32_t fd, void* buf, int32_t nbytes)
*   Inputs: fd - file descriptor index, buf - the buffer, and nbytes - number of bytes
*
*   Return Value: return correct value of read
*	Function: Will read using the correct read type function into the buffer
*/
int32_t 
read(int32_t fd, void* buf, int32_t nbytes) {
  pcb_t * pcb_pointer;

  //Check bounds and conditions
  if( buf == NULL || fd >= MAX_OPEN_FILE || fd < 0)
      return -1;

  //Get pcb pointer and check if its being open
  pcb_pointer = get_pcb_ptr();
  if( pcb_pointer->f_descs[fd].flags != FLAG_ACTIVE)
      return -1;

  //Obtain the correct read format
  int32_t get_correct_read = pcb_pointer->f_descs[fd].fops_jmp_tb_ptr->read(fd, buf ,nbytes);
  return get_correct_read;
}

/*
* int32_t write(int32_t fd, const void* buf, int32_t nbytes)
*   Inputs: fd - file descriptor index, buf - the buffer, and nbytes - number of bytes
*
*   Return Value: return correct value of write
*	Function: Will write using the correct write type function 
*/
int32_t 
write(int32_t fd, const void* buf, int32_t nbytes) {
    pcb_t * pcb_pointer;

    //Check bounds and conditions
    if( buf == NULL || fd >= MAX_OPEN_FILE || fd < 0)
        return -1;

    //Get pcb pointer and check if its being open
    pcb_pointer = get_pcb_ptr();
    if( pcb_pointer->f_descs[fd].flags != FLAG_ACTIVE)
        return -1;

	//Obtain the correct write format
	int32_t get_correct_write = pcb_pointer->f_descs[fd].fops_jmp_tb_ptr->write(fd, buf ,nbytes);
	return get_correct_write;
}

/*
* int32_t close(int32_t fd)
*   Inputs: fd - file descriptor index
*
*   Return Value: return correct value of close
*	Function: Will close using the correct close type function 
*/
int32_t
close(int32_t fd) {
  pcb_t * pcb_pointer;

  //Check bounds and conditions
    if(fd >= MAX_OPEN_FILE || fd < 0)
    {
        return -1;
    }

    if(fd == FD_STDIN || fd == FD_STDOUT)
    {
        return -1;
    }
  //Get pcb pointer and check if its being open
  pcb_pointer = get_pcb_ptr();
  if( pcb_pointer->f_descs[fd].flags != FLAG_ACTIVE)
    return -1;
  else
    pcb_pointer->f_descs[fd].flags = FLAG_INACTIVE;

  //Obtain the correct close format
  int32_t get_correct_close = pcb_pointer->f_descs[fd].fops_jmp_tb_ptr->close(fd);
  return get_correct_close;
}

//worry later
int32_t 
getargs(uint8_t* buf, int32_t nbytes) {
    if(buf == NULL)
    {
        return -1;
    }

    pcb_t * getargs_pcb; 
    getargs_pcb = (pcb_t *) (PAGE_8MB-STACK_8KB*(active_proc_num+1));

    if(nbytes < (uint32_t)strlen((int8_t *)getargs_pcb->arg_buff))
    {
        return -1;
    }

    strcpy((int8_t *)buf, (int8_t *)getargs_pcb->arg_buff);

	return 0;
}

//vidmap function
//This function maps user-level video memory to kernel-level video memory
//Input: screen_start - pointer to the pointer that wants to be mapped
//Return: The virtual video memory for user level
int32_t 
vidmap(uint8_t** screen_start){
  if((uint32_t)screen_start<USER_IMG_START||(uint32_t)screen_start>=USER_IMG_END)
    return -1;
  new_userVID_page((uint8_t*)USER_IMG_END); //132MB
  *screen_start = (uint8_t*)USER_IMG_END;
  return USER_IMG_END;
}

/* Following two are for extra credit and later*/
int32_t 
set_handler (int32_t signum, void* handler_address) {
	return -1;
}
int32_t 
sigreturn (void) {
	return -1;
}

/*
* int32_t gen_new_proc_id(void)
*   Inputs: none
*
*   Return Value: return the proccess id
*	Function: s
*/
int32_t
gen_new_proc_id(void) {
    int32_t i;

    //Look through the array to see if any process is available to be used
    for(i=0; i<MAX_PROCESSES;i++)
    {
        if(proc_id_flags[i]==0)
        {
            proc_id_flags[i] = 1;
            return i;
        }
    }
    return (-1);
}

/*
* pcb_t * get_pcb_ptr()
*   Inputs: none
*
*   Return Value: return the pointer to the pcb
*	Function: get the pointer to pcb
*/
pcb_t *
get_pcb_ptr(){
  pcb_t * pcb_ptr;
  asm("andl %%esp, %1 \n"
      "movl %1, %0 \n"
      :"=r"(pcb_ptr)
      :"r"(PCB_MASK)
      :"cc"
      );
  return pcb_ptr;
  //return (pcb_t *)(PAGE_8MB-STACK_8KB*(active_proc_num+1));
}

/*
* int32_t do_nothing() 
*   Inputs: none
*
*   Return Value: -1
*	Function: Does nothing but returns -1 to indicate not doing anything
*/
int32_t 
do_nothing() {
	return -1;
}



