/* 
 * Keyboard.c will retrive the key from input and print it out on screen after mapping the scancode
 */

#include "keyboard.h"
#include "lib.h"
#include "i8259.h"
#include "system_call.h"
#include "terminal.h" 
#include "paging.h"

//testing
#include "rtc.h"
 
// See scancode_array, value correspond to scancode_array's of either (0,1,2,3)
static uint8_t keyboard_mode = PRESS_NOTHING;			// Initial value is 0.
static uint8_t ctrl_flag = PRESS_NOTHING;				// Initial value is 0.
static uint8_t alt_flag = PRESS_NOTHING;				// Initial value is 0.

static volatile uint8_t buffer_key0[KEYBOARD_NUM_KEYS];			//Buffer that stores all the key pulled up to 128 characters
static volatile uint8_t buffer_key1[KEYBOARD_NUM_KEYS];			//Buffer that stores all the key pulled up to 128 characters
static volatile uint8_t buffer_key2[KEYBOARD_NUM_KEYS];			//Buffer that stores all the key pulled up to 128 characters
static volatile uint8_t* buffKeyPtr[3] = {buffer_key0,buffer_key1,buffer_key2};
static volatile uint8_t buffer_index;						//Index of after buffer's added key
static volatile uint8_t return_flag;
static volatile uint8_t cur_exec_term;
term_t terminals[NUM_TERM];

static uint8_t cur_term_id;

int32_t terminal_get_id(){
  
  return 0;
}

//The array which maps the scancode to the actual key depending on the mode it is in.
static uint8_t scancode_array[KEYBOARD_MODE_SIZE][KEYBOARD_NUM_KEYS] = {
	
	// no shift pressed, no caps pressed (0), PRESS_NOTHING
	{	
		//Error code, Esc, 1-9, backspace
		'\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\0',
		//tab, q-p, [, ], Enter, Ctrl
		'\0', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\0', '\0',
		//a-l, ;, ', `, L shift, \, z,
		'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', '\0', '\\', 'z',
		// x-m, ,, ., /, R shift, *PrtSc, alt, space, caps, F1
		'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', '0', '*', '\0', ' ', '\0', '\0',
		//F2-F10, NUM Lock, Scroll Lock, home, up
		'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', 
		//pgup, - , left, center, right, +, end, down, pgdn, ins, del, /, enter, F11
		'\0', '-', '\0', '\0', '\0', '+', '\0', '\0', '\0', '\0', '\0', '/', 
		//F12, Remaining/repeating undefined keys (>=87)
		'\0', '\0' 
	},
	// shift pressed, no caps pressed (1), PRESS_SHIFT_ONLY
	{
		
		'\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\0',
		'\0', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\0', '\0',
		'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', '\0', '|', 'Z',
		'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', '0', '*', '\0', ' ', '\0', '\0',
		'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', 
		'\0', '-', '\0', '\0', '\0', '+', '\0', '\0', '\0', '\0', '\0', '/', 
		'\0', '\0' 
	},
	// no shift pressed, caps pressed (2), PRESS_CAP_ONLY
	{
		'\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\0',
		'\0', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\0', '\0',
		'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', '\0', '\\', 'Z',
		'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', '0', '*', '\0', ' ', '\0', '\0',
		'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', 
		'\0', '-', '\0', '\0', '\0', '+', '\0', '\0', '\0', '\0', '\0', '/', 
		'\0', '\0' 
	},
	//  shift pressed, caps pressed (3), PRESS_SHIFT_CAP		
	{
		'\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\0',
		'\0', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\0', '\0',
		'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':', '"', '~', '\0', '|', 'z',
		'x', 'c', 'v', 'b', 'n', 'm', '<', '>', '?', '0', '*', '\0', ' ', '\0', '\0',
		'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', 
		'\0', '-', '\0', '\0', '\0', '+', '\0', '\0', '\0', '\0', '\0', '/', 
		'\0', '\0' 
	}
};

/************************ TERMINAL STUFFS**************************/
/* init_terminals
*
*
*
*/
void init_terminals(){
    int i,k;

    for( i = 0; i< NUM_TERM; i++)
    {
        terminals[i].term_id = i;
        terminals[i].active_flag = NOT_ACTIVE;
        terminals[i].x = terminals[i].y = 0;
        terminals[i].buffer_index = 0;
        terminals[i].is_in_use_flag = 0;
        terminals[i].term_vid_mem = (uint8_t*)TERM_VIR_VID;
        //TODO: PAGING CRAP HERE
        mapTermVID(i);

        //Set the color of the whole screen for that terminal
        for(k = 0; k < NUM_ROWS*NUM_COLS; k++)
          *(uint8_t *)(terminals[i].term_vid_mem + (k << 1) + 1) = TERMINAL_ATTRIB;
    }

    //terminal[TERM_0].

    //First terminal setup
    buffer_index = terminals[TERMINAL_ID0].buffer_index;
    cur_term_id = TERMINAL_ID0;

    set_coordX(terminals[TERMINAL_ID0].x);
    set_coordY(terminals[TERMINAL_ID0].y);
    terminals[TERMINAL_ID0].is_in_use_flag = ACTIVE;
    terminals[TERMINAL_ID0].active_flag = ACTIVE;
    memcpy( (uint8_t *)VIDEO, (uint8_t *) terminals[TERMINAL_ID0].term_vid_mem, _4KB);

    //terminal_restore(0);
    execute((uint8_t*)"shell");
}

/* init_terminals
*
*
*
*/

int32_t terminal_restore(uint8_t terminal_id) {

    if(terminal_id>= NUM_TERM)
    {
        return -1;
    }
    //Restore the x and y cursor
    set_coordX(terminals[terminal_id].x);
    set_coordY(terminals[terminal_id].y);

    //Restore the smallest amount of neccesarily required video memory page
    mapTermVID(terminal_id);
    memcpy( (uint8_t *)VIDEO, (uint8_t *) terminals[terminal_id].term_vid_mem, _4KB);

    //Save the index of the terminal
    buffer_index = terminals[terminal_id].buffer_index;

    return 0; 
}

int32_t terminal_save(uint8_t terminal_id) {

    if(terminal_id>= NUM_TERM)
    {
        return -1;
    }
    //Save the x and y cursor
    terminals[terminal_id].x = get_coordX();
    terminals[terminal_id].y = get_coordY();

    //Save the index of the terminal
    terminals[terminal_id].buffer_index = buffer_index; 

    //Copy over the smallest amount of neccesarily required video memory page
    mapTermVID(terminal_id);
    memcpy( (uint8_t *) terminals[terminal_id].term_vid_mem, (uint8_t *)VIDEO, _4KB);

    return 0;
}

int32_t terminal_switch_term(uint8_t target_terminal_id)
{
    //Save the previous terminal information and check 
    if(target_terminal_id == cur_term_id)
    {
        return 0;
    }
    if(target_terminal_id >= NUM_TERM || cur_term_id >= NUM_TERM)
    {
        printf("swtich fail");
        return -1;
    }

    int32_t save_complete = terminal_save(cur_term_id);
    if(save_complete == -1)
        return -1;

    //Restore the previous terminal information and check 
    int32_t restore_complete = terminal_restore(target_terminal_id);
    if(restore_complete == -1)
        return -1;

    return 0;
}

int32_t terminal_launch(uint8_t target_terminal_id)
{
    

    if(target_terminal_id == cur_term_id)
    {
        return 0;
    }
    if(target_terminal_id >= NUM_TERM || cur_term_id >= NUM_TERM)
    {
        printf("launch fail");
        return -1;
    }
    
    int32_t save_complete = terminal_save(cur_term_id);
    if(save_complete == -1)
        return -1;
    terminals[target_terminal_id].is_in_use_flag = ACTIVE;
    cur_term_id = target_terminal_id;

    sti();
    execute((uint8_t*)"shell");
    return 0;
}

int32_t terminal_change(uint8_t target_terminal_id)
{
    if(target_terminal_id >= NUM_TERM)
    {
        sti();
        return -1;
    }
    if(terminal_switch_term(target_terminal_id)==-1)
    {
        sti();
        return -1;
    }
    cur_term_id = target_terminal_id;
    mapTermVID(target_terminal_id);
    sti();
  return 0;
}

int32_t terminal_LoS(uint8_t target_terminal_id)
{
    cli();
    if (target_terminal_id >= NUM_TERM)
    {
        sti();
        return -1;
    }

    if (target_terminal_id == cur_term_id)
    {
        sti();
        return 0;
    }

    if (terminals[target_terminal_id].is_in_use_flag == ACTIVE)
        return terminal_change(target_terminal_id);
    else
        return terminal_launch(target_terminal_id);
}



/*
* int32_t open_keyboard()
*   Inputs: none
*
*   Return Value: NOTHING
*	Function: Initialize keyboard with the KEYBOARD_IRQ on the PIC
*   Also set up the buffer for the keyboard
*/
int32_t
open_keyboard(const uint8_t* filename){
	initialize_clear_buffer();
	enable_irq(KEYBOARD_IRQ);
  return 0;
}

/*
* int32_t close_keyboard(){
*   Inputs: none
*
*   Return Value: NOTHING
* Function: close the keyboard with the KEYBOARD_IRQ on the PIC
*/
int32_t
close_keyboard(int32_t fd){
  disable_irq(KEYBOARD_IRQ);
  return 0;
}

/*
* void keyboard_int_handler(){
*   Inputs: none
*
*   Return Value: NOTHING
*	Function: Will retrive a scancode from the keyboard addr port and check each case
*   to make sure the function is an enter, caps, shift, or other character.
*/
void
keyboard_int_handler(){
	//cli();
	disable_irq(KEYBOARD_IRQ);
  //sti();
  
	uint8_t key;
	while(1)
	{
		if( inb(KEYBOARD_ADDR_PORT) != 0)
		{
			//Get a key input from the keyboard buffer
			key = inb(KEYBOARD_ADDR_PORT);
			break;
		}
	}

	//Perform a certain function depending on the key retrieved
	switch(key){
		case CAPS_DOWN:
			press_caps();
			break;
		case ENTER:
			press_enter();
			break;	
		case LEFT_SHIFT_DOWN:
			press_shift();
			break;
		case LEFT_SHIFT_UP:
			unpress_shift();
			break;
		case RIGHT_SHIFT_DOWN:
			press_shift();
			break;
		case RIGHT_SHIFT_UP:
			unpress_shift();
			break;
		case BKSP:
			press_bskp();
			break;
		case CTRL_DOWN:
			set_ctrl_flag(CTRL_DOWN);
			break;
		case CTRL_UP:
			set_ctrl_flag(CTRL_UP);
			break;	
		case ALT_DOWN:
			set_alt_flag(ALT_DOWN);
			break;	
		case ALT_UP:
			set_alt_flag(ALT_UP);
			break;	
    case F1_DOWN:
      if( alt_flag == PRESS_ALT)
      {
        send_eoi(KEYBOARD_IRQ);
        terminal_LoS(TERMINAL_ID0);
      }
      break;
    case F2_DOWN:
      if( alt_flag == PRESS_ALT)
      {
        send_eoi(KEYBOARD_IRQ);
        terminal_LoS(TERMINAL_ID1);
      }
      break;  
    case F3_DOWN:
      if( alt_flag == PRESS_ALT)
      {
        send_eoi(KEYBOARD_IRQ);
        terminal_LoS(TERMINAL_ID2);
      }
      break;
		default:
			press_other_key(key);
			break;
	}
	
	//send end of interrupt	
	send_eoi(KEYBOARD_IRQ);
	
  //cli();
	enable_irq(KEYBOARD_IRQ);
  //sti();
}

/*
* void press_caps(){
*   Inputs: none
*
*   Return Value: NOTHING
*	Function: Will modify the keyboard mode to reflict the all caps being turn on or off
*/
void
press_caps(){
	//Check for the each cases of the keyboard mode and modify it according to caps being turn off or on
	switch(keyboard_mode) {
		case PRESS_CAP_ONLY:
			keyboard_mode = PRESS_NOTHING;
			break;
		case PRESS_SHIFT_ONLY:
			keyboard_mode = PRESS_SHIFT_CAP;
			break;
		case PRESS_SHIFT_CAP:
			keyboard_mode = PRESS_SHIFT_ONLY;
			break;
		default:
			keyboard_mode = PRESS_CAP_ONLY;
			break;		
	}

	/* Old one
	if(keyboard_mode == PRESS_NOTHING)
		keyboard_mode = PRESS_CAP_ONLY;
	else if( keyboard_mode == PRESS_SHIFT_ONLY )
		keyboard_mode = PRESS_SHIFT_CAP;
	else if( keyboard_mode == PRESS_SHIFT_CAP)
		keyboard_mode = PRESS_SHIFT_ONLY;
	else
		keyboard_mode = PRESS_NOTHING;
	*/
}

/*
* void press_enter()
*   Inputs: none
*
*   Return Value: NOTHING
*	Function: Set the next key on the buffer to be null for keyboard_read.
*	Reset the buffer index and print out the character new line to shift to the next line.
*/
void
press_enter() {
  //Set key to null to terminate at null for other function anc reset buffer index
  //buffer_key[buffer_index] = KEY_NULL;
  buffKeyPtr[cur_term_id][buffer_index] = KEY_NULL;
  initialize_clear_buffer();
  //Move the cursor to the next line and the screen positions
  putc(NEW_LINE);
  return_flag = 1;
}

/*
* void press_shift(){
*   Inputs: none
*
*   Return Value: NOTHING
*	Function: Will modify the keyboard mode to reflict the add the shift
*/
void 
press_shift(){
	//Change the keyboard mode to apply shift
	if(keyboard_mode == PRESS_CAP_ONLY)
		keyboard_mode = PRESS_SHIFT_CAP;
	else
		keyboard_mode = PRESS_SHIFT_ONLY;	

}

/*
* void unpress_shift(){
*   Inputs: none
*
*   Return Value: NOTHING
*	Function: Will modify the keyboard mode to reflict the remove of shift
*/
void 
unpress_shift(){
	//Change the keyboard mode to remove shift
	if(keyboard_mode == PRESS_SHIFT_CAP)
		keyboard_mode = PRESS_CAP_ONLY;
	else
		keyboard_mode = PRESS_NOTHING;	

}

/*
* void press_bskp(){
*   Inputs: none
*   Return Value: NOTHING
*	Function: Will modify the buffer array and index and the screen
*/
void
press_bskp() {
  if( buffer_index  > 0)
  {
    int x; 
    int y;

    //Move back to the last key and make it a null key 
    //buffer_key[buffer_index-1] = KEY_NULL;
    buffKeyPtr[cur_term_id][buffer_index-1] = KEY_NULL;
    buffer_index = buffer_index - 1;

    x = get_coordX();
    y = get_coordY();

    if( y>1|| x>0 ) 
    {
    	//Move back one space on video memory display
      	*(uint8_t *)(VIDEO + ((NUM_COLS*y + x-1) << 1)) = ' ';
        *(uint8_t *)(VIDEO + ((NUM_COLS*y + x-1) << 1) + 1) = ATTRIB;
    }

    if( x != 0)
     	set_coordX(x-1);
    else
    {	
      	if(y>0)
        //Move back one row
        set_coordY(y-1);

      	//NUM_COLS was already defined for us in lib.c
     	//Move back one col
      	set_coordX(NUM_COLS-1);
    }
  }
  //update curser after changing the position
  move_curser();
}

/*
* void press_other_key(uint8_t key){
*   Inputs: uint8_t key which is the scancode take from the keyboard addr port
*
*   Return Value: NOTHING
*	Function: Will print out the character to the screen
*/
void
press_other_key(uint8_t key){

  uint8_t actual_key = 0;
  //int32_t fd = -1;
  //uint8_t* null_filename = NULL;
  
  //for testing
  //static uint32_t mul2 = 2;

  //If key is not in the keyboard array than it does not need to be consider
  if(NUM_ACTUAL_MAP_KEYS <= key)
    return;

  //Get the key that is being map to scancode_array
  actual_key = scancode_array[keyboard_mode][key];

  if(actual_key != KEY_NULL)
  {
    if(ctrl_flag == PRESS_NOTHING){
      if(buffer_index != KEYBOARD_NUM_KEYS)
      {
        //add the key to the buffer
        //buffer_key[buffer_index] = actual_key;
        buffKeyPtr[cur_term_id][buffer_index] = actual_key;
        buffer_index += 1;
        //print a key to the screen
        putc(actual_key);
      }
    }
    else
    {
      if( (actual_key == 'l') || (actual_key == 'L') )
      {
        //clear screen video memory
        clear();
        initialize_clear_buffer();
        //Set the Coordinate of x and y to be zero for the screen
        set_coordY(Y_ZERO);
        set_coordX(X_ZERO);
        move_curser();
      }
      //Will perform ctrl + c
      if( (actual_key == 'c') || (actual_key == 'C') )
      {
        halt(0);
      }
      /*
      //for testing Sandwich
      if( (actual_key == 'w') || (actual_key == 'W') )
      {
        if(mul2<1024)
          mul2 *= 2;
        rtc_write(fd,&mul2,4);
      }
      //for testing
      if( (actual_key == 'S') || (actual_key == 's') )
      {
        if(mul2>2)
          mul2 /= 2;
        rtc_write(fd,&mul2,4);
      }
      if((actual_key == '4')){
        clear();
        //initialize_clear_buffer();
        //Set the Coordinate of x and y to be zero for the screen
        set_coordY(Y_ZERO);
        set_coordX(X_ZERO);
        move_curser();
        rtc_open(null_filename);
        //rtc_write(&mul2,4);
      }
      if((actual_key == '5')){
        rtc_close(fd);
      }
      //for testing
      if((actual_key == '6')){
        int i;
        char test_text[30] = "Test read keyboard buffer";
        char my_test_buf[128];
        clear();
        set_coordY(Y_ZERO);
        set_coordX(X_ZERO);
        move_curser();
        keyboard_write(fd, &test_text,25);
        putc(NEW_LINE);
        
        keyboard_read(fd, my_test_buf,128);
        for(i=0;i<buffer_index&&my_test_buf[i]!=KEY_NULL;i++)
          putc(my_test_buf[i]);
        
      }
      //While loop inside interrupt is crashing the system
      if((actual_key == '7')){
        clear();
        set_coordY(Y_ZERO);
        set_coordX(X_ZERO);
        move_curser();
        char test_text[30] = "Waiting for RTC interrupt";
        char response_text[30] = "RTC interrupt occurred";
        keyboard_write(fd, &test_text,25);
        putc(NEW_LINE);
        rtc_read();
        keyboard_write(fd, &response_text,22);
      }*/
    }
  }

  return;
}

/*
* void set_clear_buffer() 
*   Inputs: none
*   Return Value: NOTHING
*	Function: Set the whole buffer to null key and set the index to 0
*/
void
initialize_clear_buffer() {
  //int i;

  //Set the buffer index to the beginning
  buffer_index = 0;

  /*
  for(i = 0; i < KEYBOARD_NUM_KEYS; i++)
  {
    //Set the whole buffer to null key 
    buffer_key[i] = KEY_NULL;
    
  }*/
}

/*
* void set_ctrl_flag(uint8_t key)
*   Inputs: the key for either ctrl press down or up
*   Return Value: NOTHING
*	Function: Set the ctrl_flag base on the key input.
*   Flag = 0 means not pressed
*	Flag = 4 mean pressed
*/
void
set_ctrl_flag(uint8_t key) {
	//Set ctrl_flag base on key input.
	if(key == CTRL_DOWN)
		ctrl_flag = PRESS_CTRL;
	if(key == CTRL_UP)
		ctrl_flag = PRESS_NOTHING;
}

/*
* void set_alt_flag(uint8_t key) 
*   Inputs: the key for either alt press down or up
*   Return Value: NOTHING
*	Function: Set the alt_flag base on the key input. 
*   Flag = 0 means not pressed
*	Flag = 5 mean pressed
*/
void
set_alt_flag(uint8_t key) {
	//Set ctrl_flag base on key input.
	if(key == ALT_DOWN)
		alt_flag = PRESS_ALT;
	if(key == ALT_UP)
		alt_flag = PRESS_NOTHING;
}

/*
* int32_t keyboard_read(int32_t fd, void * buff, int32_t nbytes) 
*   Inputs: void * buff: the buffer that needs to be copy to, 
*			int32_t nbytes: number of bytes to copy
*   Return Value: The number of keys copied
*	Function: Copy data from one line that has been terminated by press Enter, or as much as fits in the buffer from one such line.
*   Afterward return the number of keys copied.
*/
int32_t 
keyboard_read(int32_t fd, void* buf, int32_t nbytes){
  int i;
  return_flag = 0;
  while(!return_flag);
  for(i=0; i<KEYBOARD_NUM_KEYS; i++) {
    //Only copy the key if it is not null or up to nbytes
    if(i>=nbytes)
      return i;
    //Copy the key from buffer to the buff
    *(unsigned char*)(buf+i) = buffKeyPtr[cur_term_id][i];//buffer_key[i];
    if(buffKeyPtr[cur_term_id][i] == KEY_NULL)//buffer_key[i] == KEY_NULL)
      return i;
  }

  return 0;
}

/*
* int32_t keyboard_write(int32_t fd, void * buff, int32_t nbytes)
*   Inputs: void * buff: the buffer that will be print to the terminal
*			int32_t nbytes: number of bytes to print to terminal
*   Return Value: The number of keys printed
*	Function: Keep printing the keys in the buff until the nbytes is satisfied
*   Afterward return the number of keys printed.
*/
int32_t 
keyboard_write(int32_t fd, const void* buff, int32_t nbytes){
  int i;
  /*for(i=0; i<nbytes; i++){
    buffer_key[i] = *(unsigned char*)(buff+i);
  }
  buffer_index = i;
  if(i<KEYBOARD_NUM_KEYS-1)
    buffer_key[i] = KEY_NULL;*/
  for(i=0; i<nbytes; i++){
    putc(*(unsigned char*)(buff+i));
  }
  return i;
}




