/* Adding all the possible includes that we might need */
#include "system_call.h"
#include "file_sys_module.h"

int32_t 
halt(uint8_t status) {

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
    uint8_t parsed_fname[MAX_FILE_SIZE];
    uint8_t parsed_arg[]
    int fname_start, fname end;




    sti();
}

int32_t 
read(int32_t fd, void* buf, int32_t nbytes) {

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
	
*/