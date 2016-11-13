/* Adding all the possible includes that we might need */
#include "system_call.h"
#include "file_sys_module.h"

static int proc_id_flags[MAX_PROCESSES] = {0,0,0,0,0,0,0};

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
    int8_t f_content_buf[MIN_READ_ELF_SIZE];
	int8_t bitmask;
    dentry_t f_dentry;
    int32_t f_content = 0;
    uint32_t entry_point;
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

    //Begin populating the PCB
    pcb_t * 

    //obtain entry point
    entry_point = (uint32_t) f_content;

	bitmask = 0x80;

	for(i = 0; i < MAX_OPEN_FILE; i++)
	{
		
	}

    sti();
    return 0;
}

int32_t 
read(int32_t fd, void* buf, int32_t nbytes) {

	sti();

	//Need to set up PCB

	//Check bounds
	if( fd > 7 || fd < 0)
		return -1;
	if( buf == NULL )
		return -1;
	return 0;
}

int32_t 
write(int32_t fd, const void* buf, int32_t nbytes) {
	return 0;
}

int32_t 
open(const uint8_t* filename) {
	return 0;
}

int32_t
close(int32_t fd) {
	return 0;
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
    int8_t i, proc_id;
    int8_t avail_process_flag = 0;


    for(i=0; i<MAX_PROCESSES;i++)
    {
        if(proc_id_flags[i]==0)
        {
            proc_id_flags[i] = 1;
            avail_process_flag = 1;
            proc_id = i;
        }
    }
    return (avail_process_flag == 1 ? proc_id:-1)
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
/*
Hey Herman, 
	This is what you need to know:
	In the google doc there are some information on 3.3.
	Aaron worked on file system and pcb.
	Phong started read and write. 
	Meet tomorrow 10 am at grainger
*/




