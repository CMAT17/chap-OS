/* Adding all the possible includes that we might need */
#include "system_call.h"


int32_t 
halt(uint8_t status) {

}

int32_t 
execute(const uint8_t* command) {

	int i = 0;
	int name_starting_point = 0;
	int name_ending_point = 0;
	int arg_starting_point = 0;
	int arg_ending_point = 0;
	int8_t file_name_command[MAX_NAME_SIZE];
	int8_t arg_command[MAX_ARG_SIZE];
	int8_t bitmask;

	// The command does not exist
	if( command == NULL )
		return -1;

	// Parsing the Program Name
	i = 0;

	//Traverse through the command for the name and argument
	while(command[i] != NULL_CHAR)
	{
		if( name_starting_point == 0)
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
		else if( name_ending_point == 0)
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
		else if( arg_starting_point == 0)
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
		else if( arg_ending_point == 0)
		{
			if( command[i] != ' ')
			{
				arg_command[i-arg_starting_point] = (int8_t)command[i];
				i++;
			}
			else
			{
				arg_ending_point = i;
				file_name_command[i] = NULL_CHAR;
				break;
			}
		}
		else
			return -1;
	}

	//Read the 4 bytes from the file to the buffer?
	if( read_data(inode offset, buff, length))

	//Check if there is an executable images?
	//use strncmp	

	bitmask = 0x80;

	for(i = 0; i < MAX_OPEN_FILE; i++)
	{
		
	}
}

int32_t 
read(int32_t fd, void* buf, int32_t nbytes) {

	sti();

	//Need to set up PCB

	//Check bounds
	if( fd > 7 || fd < 0)
		return -1;
	if( buf == NULL )
		retun -1;
}

int32_t 
write(int32_t fd, const void* buf, int32_t nbytes) {

}

int32_t 
open(const uint8_t* filename) {

}

int32_t
close(int32_t fd) {

}

int32_t 
getargs(uint8_t* buf, int32_t nbytes) {

}

int32_t 
vidmap(uint8_t** screen_start) {

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
Hey Herman, 
	This is what you need to know:
	In the google doc there are some information on 3.3.
	Aaron worked on file system and pcb.
	Phong started read and write. 
	Meet tomorrow 10 am at grainger
*/