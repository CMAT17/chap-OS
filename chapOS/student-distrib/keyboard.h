/* 
 * Keyboard.h initialize a lof the of the constants need to map the scancode to the key
 */

#ifndef _KEYBOARD_H
#define	_KEYBOARD_H

#include "types.h"

#define	KEYBOARD_ADDR_PORT 	0x60
#define KEYBOARD_IRQ		1

//Magic Numbers
#define	KEYBOARD_MODE_SIZE	4
#define KEYBOARD_NUM_KEYS	128
#define NUM_ACTUAL_MAP_KEYS 87 
#define CAPS_DOWN 			0x3A
#define	ENTER				0x1C
#define	LEFT_SHIFT_DOWN		0x2A
#define LEFT_SHIFT_UP		0xAA
#define	RIGHT_SHIFT_DOWN	0x36
#define RIGHT_SHIFT_UP		0xB6
#define BKSP				0x0E
#define CTRL_DOWN			0x1D
#define CTRL_UP 			0x9D
#define F1_DOWN				0x3B
#define F2_DOWN	 			0x3C
#define	F3_DOWN  			0x3D 
#define ALT_DOWN			0x38
#define ALT_UP 				0xB8
 			 
#define PRESS_NOTHING		0
#define	PRESS_SHIFT_ONLY	1
#define	PRESS_CAP_ONLY		2
#define	PRESS_SHIFT_CAP		3 

#define PRESS_CTRL			4 
#define PRESS_ALT			5 
#define	KEY_NULL    		'\0'
#define NEW_LINE			'\n' 

#define X_ZERO				0
#define Y_ZERO				0 
#define	_4KB				0x1000  

#define TERMINAL_ID0		0
#define	TERMINAL_ID1 		1
#define	TERMINAL_ID2		2 
#define TERMINAL_ATTRIB		0x02 
#define TERM_VIR_VID    0x08401000

//Initialize keyboard with the KEYBOARD_IRQ on the PIC
int32_t open_keyboard( const uint8_t* filename );

//Will retrive a scancode from the keyboard addr port and check each case
//to make sure the function is an enter, caps, shift, or other character.
void keyboard_int_handler();

//Will modify the keyboard mode to reflict the all caps being turn on or off
void press_caps();

//Will modify the keyboard mode to reflict the add the shift
void press_shift();

//Will modify the keyboard mode to reflict the remove of shift
void unpress_shift();

//Will shift the screen x and y coordinate down one line and clear buffer
void press_enter();

// Will modify the buffer array and index and the screen
void press_bskp();

//Will print out the character to the screen
void press_other_key(uint8_t key);

//Set the whole buffer to null key and set the index to 0
void initialize_clear_buffer();

//Set the ctrl_flag base on the key input.
void set_ctrl_flag(uint8_t key);

//Set the alt_flag base on the key input. 
void set_alt_flag(uint8_t key);

//Function: Copy data from one line that has been terminated by press Enter, or as much as fits in the buffer from one such line.
//Afterward return the number of keys copied.
int32_t keyboard_read(int32_t fd, void* buf, int32_t nbytes);

//Function: Keep printing the keys in the buff until the nbytes is satisfied
//Afterward return the number of keys printed.
int32_t keyboard_write(int32_t fd, const void* buff, int32_t nbytes);

//Function: Close the keyboard
//Afterward no return value.
int32_t close_keyboard(int32_t fd);


//Function: Change the value of cur_term_id
void set_cur_term_id(uint8_t new_id);

int32_t terminal_restore(uint8_t terminal_id);


#endif /* end of _KEYBOARD_H */
