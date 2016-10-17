/* 
 * Keyboard.c will retrive the key from input and print it out on screen after mapping the scancode
 */

#include "keyboard.h"
#include "lib.h"
#include "i8259.h"

// See scancode_array, value correspond to scancode_array's of either (0,1,2,3)
// Initial value is 0. 
static uint8_t keyboard_mode = PRESS_NOTHING;

volatile uint8_t * buffer_key;			//Buffer that stores all the key pulled
volatile uint8_t buffer_index = 0;		//Index of buffer's last added key

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
* void initialize_keyboard()
*   Inputs: none
*
*   Return Value: NOTHING
*	Function: Initialize keyboard with the KEYBOARD_IRQ on the PIC
*/
void
initialize_keyboard(){
	enable_irq(KEYBOARD_IRQ);
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
	
	disable_irq(KEYBOARD_IRQ);

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
		/*case ENTER:
			press_enter();
			break;	*/	
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
		/*case BKSP:
			press_bskp();
			break;*/	
		default:
			press_other_key(key);
			break;
	}
	
	//send end of interrupt	
	send_eoi(KEYBOARD_IRQ);
	
	enable_irq(KEYBOARD_IRQ);
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
	if(keyboard_mode == PRESS_NOTHING)
		keyboard_mode = PRESS_CAP_ONLY;
	else if( keyboard_mode == PRESS_SHIFT_ONLY )
		keyboard_mode = PRESS_SHIFT_CAP;
	else if( keyboard_mode == PRESS_SHIFT_CAP)
		keyboard_mode == PRESS_SHIFT_ONLY;
	else
		keyboard_mode = PRESS_NOTHING;
}

//To be done later
/*
void
press_enter(){
}*/

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

//To be done later
/*void
press_bskp(){
}*/

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

	//If key is not in the keyboard array than it does not need to be consider
	if(KEYBOARD_NUM_KEYS <= key)
		return;

	//Get the key that is being map to scancode_array
	actual_key = scancode_array[keyboard_mode][key];

	if((buffer_index < buffer_key))
	{	
		if(actual_key != KEY_NULL)
		{	
			buffer_key[buffer_index] = actual_key;
			buffer_index += 1;
			//print a key to the screen
			putc(actual_key);
		}
	}

}











