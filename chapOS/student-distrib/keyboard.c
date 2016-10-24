/* 
 * Keyboard.c will retrive the key from input and print it out on screen after mapping the scancode
 */

#include "keyboard.h"
#include "lib.h"
#include "i8259.h"

//testing
#include "rtc.h"
 
// See scancode_array, value correspond to scancode_array's of either (0,1,2,3)
static uint8_t keyboard_mode = PRESS_NOTHING;			// Initial value is 0.
static uint8_t ctrl_flag = PRESS_NOTHING;				// Initial value is 0.
static uint8_t alt_flag = PRESS_NOTHING;				// Initial value is 0.

volatile uint8_t buffer_key[KEYBOARD_NUM_KEYS];			//Buffer that stores all the key pulled up to 128 characters
volatile uint8_t buffer_index = 0;						//Index of buffer's last added key

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
		//F12, Remaining/repeating undefined keys (>=92)
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

/*
* void open_keyboard()
*   Inputs: none
*
*   Return Value: NOTHING
*	Function: Initialize keyboard with the KEYBOARD_IRQ on the PIC
*   Also set up the buffer for the keyboard
*/
void
open_keyboard(){
	initialize_clear_buffer();
	enable_irq(KEYBOARD_IRQ);
}

/*
* void close_keyboard(){
*   Inputs: none
*
*   Return Value: NOTHING
* Function: close the keyboard with the KEYBOARD_IRQ on the PIC
*/
void
close_keyboard(){
  disable_irq(KEYBOARD_IRQ);
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
	cli();
	disable_irq(KEYBOARD_IRQ);
  sti();
  
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
		default:
			press_other_key(key);
			break;
	}
	
	//send end of interrupt	
	send_eoi(KEYBOARD_IRQ);
	
  cli();
	enable_irq(KEYBOARD_IRQ);
  sti();
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


	/*if(keyboard_mode == PRESS_NOTHING)
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
*	Function: Will shift the screen x and y coordinate down one line and clear buffer
*/
void
press_enter() {


  //y = get_coordY();
  //Move to the next line for the coordinate
  //set_coordY(y+1);
  //set_coordX(X_ZERO);

  //Clear the buffer
  buffer_key[buffer_index] = KEY_NULL;
  initialize_clear_buffer();
  putc('\n');

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

    buffer_key[buffer_index-1] = KEY_NULL;
    buffer_index = buffer_index - 1;

    x = get_coordX();
    y = get_coordY();
    if(y>1||x>0){
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

  //for testing
  static uint32_t mul2 = 2;

  //If key is not in the keyboard array than it does not need to be consider
  if(KEYBOARD_NUM_KEYS <= key)
    return;

  //Get the key that is being map to scancode_array
  actual_key = scancode_array[keyboard_mode][key];

  if(actual_key != KEY_NULL)
  {
    if(ctrl_flag == PRESS_NOTHING){
      if(buffer_index != KEYBOARD_NUM_KEYS)
      {
        //add the key to the buffer
        buffer_key[buffer_index] = actual_key;
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
      
      //for testing Sandwich
      if( (actual_key == 'w') || (actual_key == 'W') )
      {
        if(mul2<1024)
          mul2 *= 2;
        rtc_write(&mul2,4);
      }
      //for testing
      if( (actual_key == 'S') || (actual_key == 's') )
      {
        if(mul2>2)
          mul2 /= 2;
        rtc_write(&mul2,4);
      }
      if((actual_key == '4')){
        clear();
        initialize_clear_buffer();
        //Set the Coordinate of x and y to be zero for the screen
        set_coordY(Y_ZERO);
        set_coordX(X_ZERO);
        move_curser();
        rtc_open();
        //rtc_write(&mul2,4);
      }
      if((actual_key == '5')){
        rtc_close();
      }
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


//Return data from one line that has been terminated by press Enter, or as much as fits in the buffer from one such line.
//The line returned should include the line feed character.
int32_t 
read_keyboard(void * buff, int32_t nbytes){
  int i;
  for(i=0;i<KEYBOARD_NUM_KEYS;i++){
    if(i>=nbytes||buffer_key[i]==KEY_NULL)
      return i;
    *(unsigned char*)(buff+i) = buffer_key[i];
  }
  return 0;
}

int32_t 
write_keyboard(void * buff, int32_t nbytes){
  int i;
  for(i=0;i<nbytes;i++){
    putc(*(unsigned char*)(buff+i));
  }
  return i;
}





