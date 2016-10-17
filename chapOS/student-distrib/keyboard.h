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
#define	ENTER				0x53
#define	LEFT_SHIFT_DOWN		0x2A
#define LEFT_SHIFT_UP		0xAA
#define	RIGHT_SHIFT_DOWN	0x36
#define RIGHT_SHIFT_UP		0xB6
#define BKSP				0x0E
#define PRESS_NOTHING		0
#define	PRESS_SHIFT_ONLY	1
#define	PRESS_CAP_ONLY		2
#define	PRESS_SHIFT_CAP		3
#define	NULL_KEY			'\0'

//Initialize keyboard with the KEYBOARD_IRQ on the PIC
void initialize_keyboard();

//Will retrive a scancode from the keyboard addr port and check each case
//to make sure the function is an enter, caps, shift, or other character.
void keyboard_int_handler();

//Will modify the keyboard mode to reflict the all caps being turn on or off
void press_caps();

//Will modify the keyboard mode to reflict the add the shift
void press_shift();

//Will modify the keyboard mode to reflict the remove of shift
void unpress_shift();

//Will print out the character to the screen
void press_other_key(uint8_t key);

#endif /* end of _KEYBOARD_H */
