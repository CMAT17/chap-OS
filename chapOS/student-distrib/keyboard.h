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
#define CAPS_DOWN 			0x3A
#define	ENTER				0x1C
#define	LEFT_SHIFT_DOWN		0x2A
#define LEFT_SHIFT_UP		0xAA
#define	RIGHT_SHIFT_DOWN	0x36
#define RIGHT_SHIFT_UP		0xB6
#define BKSP				0x0E
#define CTRL_DOWN			0x1D
#define CTRL_UP 			0x9D
#define ALT_DOWN			0x38
#define ALT_UP 				0xB8	 

#define PRESS_NOTHING		0
#define	PRESS_SHIFT_ONLY	1
#define	PRESS_CAP_ONLY		2
#define	PRESS_SHIFT_CAP		3 

#define PRESS_CTRL			4 
#define PRESS_ALT			5 
#define	KEY_NULL    		'\0' 

#define X_ZERO				0
#define Y_ZERO				0 

//Initialize keyboard with the KEYBOARD_IRQ on the PIC
void open_keyboard();

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

int32_t read_keyboard(void * buff, int32_t nbytes);

int32_t write_keyboard(void * buff, int32_t nbytes);

#endif /* end of _KEYBOARD_H */
