/* Adding all the possible includes that we might need */
#include "system_call.h"
#include "file_sys_module.h"
#include "paging.h"
#include "x86_desc.h"

static int proc_id_flags[MAX_PROCESSES] = {0,0,0,0,0,0};
uint32_t rtc__ops_tbl[NUM_OPS] = { (uint32_t)(rtc_open), (uint32_t)(rtc_read), (uint32_t)(rtc_write), (uint32_t)(rtc_close)};
uint32_t file_ops_tbl[NUM_OPS] = { (uint32_t)(file_open),(uint32_t)(file_read),(uint32_t)(file_write),(uint32_t)(file_close)};
uint32_t dir_ops_tbl[NUM_OPS] = { (uint32_t)(file_open),(uint32_t)(file_read),(uint32_t)(file_write),(uint32_t)(file_close)};
uint32_t stdin_ops_tbl[NUM_OPS] = { (uint32_t)(open_keyboard),(uint32_t)(read_keyboard),(uint32_t)(do_nothing),(uint32_t)(close_keyboard)};
uint32_t stdout_ops_tbl[NUM_OPS] = { (uint32_t)(open_keyboard),(uint32_t)(do_nothing),(uint32_t)(write_keyboard),(uint32_t)(close_keyboard)};

static pcb_t* get_pcb_ptr();

int32_t 
halt(uint8_t status) {
	return 0;
}
/* Execute()
 * 1) Parse command
 * 2) EXE check
 * 3) Reorganize virtual memory
 * 4) File Loader
 * 5) PCB
 * 6) Context/TSS switch
 * 7) IRET crap
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
    int32_t proc_stat;
	// The command does not exist
	if( command == NULL )
		return -1;

	// Parsing the Program Name
	i = 0;

	//Traverse through the command for the name and argument
	while(command[i] != NULL_CHAR)
	{
		if( name_starting_point == -1)
		{
			if( command[i] == ' ')
			{
				i++;				
			}
			else
			{
				name_starting_point = i;
			}
		} 
		else if( name_ending_point == -1)
		{
			if( command[i] != ' ')
			{
				file_name_command[i-name_starting_point] = (int8_t)command[i];
				i++;

			}
			else
			{
				name_ending_point = i;
				file_name_command[i] = NULL_CHAR;
			}
		}
		else if( arg_starting_point == -1)
		{
			if( command[i] == ' ')
			{
				i++;
			}
			else
			{
				arg_starting_point = i;
			}
		} 
		else if( arg_ending_point == -1)
		{
			if( command[i] != ' ')
			{
				arg_command[i-arg_starting_point] = (int8_t)command[i];
				i++;
			}
			else
			{

				arg_ending_point = i;
				arg_command[i-arg_starting_point] = NULL_CHAR; 
				printf("See arg Space\n");
				break;
			}
		}
		else
			return -1;


	}

	printf("The name of command: %s\n", file_name_command );
	printf("The arg of command: %s\n ", arg_command );

    //Make sure that file exists
    if(read_dentry_by_name((uint8_t *) file_name_command, & f_dentry))
    {
        printf ("Shits done fucked up\n");
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
        printf("What the fuck man \n");
        return -1;
    }

    //Get address of process PCB
    pcb_t * proc_PCB;
    
    new_proc_id = gen_new_proc_id();
    
    if(new_proc_id<0)
    {
        return -1;
    }

    proc_PCB = (pcb_t*)(PAGE_8MB-STACK_8KB*(new_proc_id + 1));
    proc_PCB->proc_num = new_proc_id;

    //obtain entry point
    entry_point = (uint32_t) f_content;

    //re-organize virtual memory
    proc_stat = new4MB_page();

    if(proc_stat == -1)
    {
        printf("Too many processes\n");
        return -1;
    }
    //Load file into virtual memory
    uint32_t len = get_file_size(f_dentry.inode_num);
    printf("%d\n", read_data(f_dentry.inode_num, 0, (uint8_t*)PROG_IMAGE_VADDR, len));

    //PCB s

    //Set the values for stdin open
    proc_PCB->f_descs[FDS_STDIN_IDX].flags = FLAG_ACTIVE;
    proc_PCB->f_descs[FDS_STDIN_IDX].fops_jmp_tb_ptr = stdin_ops_tbl;

    //Set the values for stdin open
    proc_PCB->f_descs[FDS_STDOUT_IDX].flags = FLAG_ACTIVE;
    proc_PCB->f_descs[FDS_STDOUT_IDX].fops_jmp_tb_ptr = stdout_ops_tbl;

    //set tss.ss0 and esp0 to hold kernel data segment and
    tss.ss0 = KERNEL_DS;
    tss.esp0 = PAGE_8MB-STACK_8KB*(new_proc_id+1);
    sti();

    asm volatile(
                    "cli                        \n"
                    "movw   $0x2B,%%ax          \n"   //User data segment index
                    "movw   %%ax, %%ds          \n"   //push to data segment register
                    "pushl  $0x2B               \n"   //push user data segment to stack
                    "movl   $0x083FFFFC, %%eax  \n"   //push 4-byte aligned bottom of stack
                    "pushl  %%eax               \n"   
                    "pushf                     \n"   //push eflags
                    "pushl  $0x23               \n"   //push user code segment
                    "movl   %0, %%ebx           \n"   //move entry point into ebx, to be pushed
                    "pushl  %%ebx               \n"
                    "iret                       \n"
                    "RET_FROM_IRET:             \n"
                    "leave                      \n"
                    "sti                        \n"
                    "ret                        \n"
                    : /*no outputs*/
                    : "r"(entry_point)
                    : "%ebx", "%eax"
                    );

    return 0;
}

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


int32_t 
open(const uint8_t* filename) {
	

	int i; 
	pcb_t * pcb_pointer;
	dentry_t entry; 
	uint32_t dentry_file_type = -1;

	pcb_pointer = get_pcb_ptr();


	//Find the index in which the file system is available
	i = MIN_OPEN_FILE;
	while(i)
	{	
		//Check if maximun number of file is open
		if( i == MAX_OPEN_FILE )
			return -1;

		if( pcb_pointer->f_descs[i].flags != FLAG_ACTIVE )
		{
			break;
		}

		i++;
	}

	pcb_pointer->f_descs[i].flags = FLAG_ACTIVE;
	pcb_pointer->f_descs[i].file_pos = FILE_POS;

	//Get the dentry and store it in entry
	read_dentry_by_name(filename, &entry);

	if( entry == -1)
		return -1;
	else
	{
		dentry_file_type = entry.file_type;
	}

	if( dentry_file_type == FILE_TYPE_DIR)
	{
		if(dir_open(filename) == 0)
		{	
			pcb_pointer->f_descs[i].fops_jmp_tb_ptr = dir_ops_tbl;
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
			pcb_pointer->f_descs[i].fops_jmp_tb_ptr = file_ops_tbl;
			//Inode only meaningful for file
			pcb_pointer->f_descs[i].inode = dentry_file_type.inode_num;

		}
		else
			return -1;
	}
	else if( dentry_file_type == FILE_TYPE_RTC)
	{
		if((rtc_open(filename) == 0)
		{
			pcb_pointer->f_descs[i].fops_jmp_tb_ptr = rtc__ops_tbl;
			//inode does not have meaning
			pcb_pointer->f_descs[i].inode = NULL;
		}
		else
			return -1;
	}

	//Return the index of the file system
	return i;
}

int32_t
close(int32_t fd) {

	pcb_t * pcb_pointer;

	//Check bounds and conditions
	if(fd >= MAX_OPEN_FILE || fd < 0)
		return -1;

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

int32_t 
getargs(uint8_t* buf, int32_t nbytes) {
	return 0;
}

int32_t 
vidmap(uint8_t** screen_start) {
	return 0;
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

int32_t
gen_new_proc_id(void)
{
    int32_t i, proc_id;
    int8_t avail_process_flag = 0;


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

pcb_t *
get_pcb_ptr()
{
    pcb_t * pcb_ptr;
    asm("andl %%esp, %%eax \n"
        :"=a"(pcb_ptr)
        :"a"(PCB_MASK)
        :"cc"
        );
    return pcb_ptr;
}

int32_t 
do_nothing() {
	return -1;
}
/*
Hey Herman, 
	This is what you need to know:
	In the google doc there are some information on 3.3.
	Aaron worked on file system and pcb.
	Phong started read and write. 
	Meet tomorrow 10 am at grainger
*/



