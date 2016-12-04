#include "rtc.h"
#include "lib.h"
#include "i8259.h"
#include "keyboard.h"

#define WIPE_MASK   0x80
#define DIV_AND_RS  0x2F
#define TESTA       0xA6
#define TESTB       0x68
#define WIPE_INT    0x42
#define PER_INT_EN  0x40
#define LOW_4_MASK  0x0F
#define HI_4_MASK   0xF0
#define MAX_FREQ    1024
#define MIN_FREQ    0

/************ Global variable(s) ************/

static uint8_t is_open = 0;

/* used for synchronization */
volatile uint8_t rtc_interrupt_occurred = 0;

//static uint8_t rtc_counter = 0;

void rtc_init(){
    unsigned char prv_a;
    unsigned char prv_b;

    //obtain the previous statuses of a and b
    outb(RTC_REG_A, RTC_PORT);
    prv_a = inb(RTC_CMOS);
    outb(RTC_REG_B, RTC_PORT);
    prv_b = inb(RTC_CMOS);

    //Enable RTC with divider freq 2 Hz (bits [3:0] = 1111),
    //22 stage divider (bits [6:4] = 010) initialized to 32.768 kHz,   
    outb(RTC_REG_A, RTC_PORT);
    outb((prv_a & WIPE_MASK)|DIV_AND_RS, RTC_CMOS);

    //Enable periodic interrupts from Status Reg B
    outb(RTC_REG_B, RTC_PORT);
    outb(prv_b | PER_INT_EN, RTC_CMOS);

    //open the irq line for the RTC
    //cli();
    enable_irq(RTC_IRQ);
    //sti();
}

/*  rtc_irq_handler
 *   temporary interrupt handler for RTC
 *   masks own IRQ line so as to not interrupt itself
 *   
 */
void rtc_irq_handler(){
  //Mask own irq line to prevent being interrupted by itself
  //cli();
  disable_irq(RTC_IRQ);
  //sti();
  
  //Obtain contents of CMOS register C (don't really care about contents)
  outb(INDEX_REG_C,RTC_PORT);
  inb(RTC_CMOS);

  // test_interrupts as a check
  //test_interrupts();

  if(is_open == 1)
    rtc_interrupt_occurred = 1;
  
  //end of interrupt done
  send_eoi(RTC_IRQ);
  
  
  //for testing
  //if(is_open == 1){
    //char sssss = '1';
    //write_keyboard(&sssss,1);
  //}
  
  //re-enable irq line
  //cli();
  enable_irq(RTC_IRQ);
  //sti();
  
}


/* rtc_read
 * INPUT: buf - 
 *        nbytes - 
 * OUTPUT: none
 * RETURN VALUE: 0 only after an interrupt has occurred
 */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
  rtc_interrupt_occurred = 0;
  while(rtc_interrupt_occurred==0){
    // spin
  }
  
  return 0;
}

/* rtc_write
 * INPUT: buf - 4-byte int specifying interrupt rate in Hz
 *        nbytes - number of bytes to write
 * OUTPUT: none
 * RETURN VALUE: -1 on failure
 *               nbytes - Number of bytes written
 */
int32_t rtc_write(int32_t fd, const void* freq, int32_t nbytes){
  //cli();
  disable_irq(RTC_IRQ);
  //sti();
  int frequency;
  int rate;
  unsigned char prev_reg_A;

  /* Check if invalid # of bytes to write */
  if(nbytes != 4)
    return -1;
  /* Check if pointer is null */
  if(freq == NULL)
    return -1;

  frequency = *(uint32_t*)freq;
  /* Frequency must be power of 2 */
  /* And not over 1024 Hz, or less than 0 Hz */
  if( ! ((frequency != 0 ) && ((frequency & (frequency -1)) == 0)))
    return -1;
  else if (frequency > MAX_FREQ || frequency < MIN_FREQ)
    return -1;

  /* Calculate divider from frequency */
  /* frequency is 32768 >> (rate-1), rate is 0-15, but in our case 6-15 */
  switch(frequency)
  {
    case 1024:
      rate = 6;   //Thus shifts 32768 by 5 bits for new frequency
      break;
    case 512:
      rate = 7;
      break;
    case 256:
      rate = 8;
      break;
    case 128:
      rate = 9;
      break;
    case 64:
      rate = 10;
      break;
    case 32:
      rate = 11;
      break;
    case 16:
      rate = 12;
      break;
    case 8:
      rate = 13;
      break;
    case 4:
      rate = 14;
      break;
    case 2:
      rate = 15;
      break;
  }

  /* Rate is between 2 and 15, only need low 4 bits*/
  rate &= LOW_4_MASK;
  /* Read current value of register A*/
  outb(RTC_REG_A,RTC_PORT);
  prev_reg_A = inb(RTC_CMOS);

  /* Set reg A with new frequency */
  outb(RTC_REG_A, RTC_PORT);
  /* Clear low 4 bits of prev A reg value and AND with new rate value */
  outb(((prev_reg_A & HI_4_MASK)| rate), RTC_CMOS);

  
  //cli();
  enable_irq(RTC_IRQ);
  //sti();
  /* Returns number of bytes written */
  return nbytes;
}

/* rtc_open
 *  Calls rtc_init, which initializes rtc and sets rate to 2 Hz
 * INPUT: none 
 * OUTPUT: none
 * RETURN VALUE: 0 - if successful
 */
int32_t rtc_open(const uint8_t* filename){
  /*if(!is_open){
    rtc_init();
    is_open = 1;
  }
  else{
    enable_irq(RTC_IRQ);
  }*/
  //cli();
  //enable_irq(RTC_IRQ);
  //sti();
  is_open = 1;
  return 0;
}

/* rtc_close
 * INPUT: none
 * OUTPUT: none
 * RETURN VALUE: 0 if successful
 */
int32_t rtc_close(int32_t fd){
  //Mask own irq line to prevent being interrupted by itself
  //cli();
  //disable_irq(RTC_IRQ);
  //sti();
  is_open = 0;
  return 0;
}




//-------------------------Start Sandwich Part---------------------------------
/*
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
  rtc_interrupt_occurred = 0;
  while(rtc_interrupt_occurred==0);
  return 0;
}
int32_t rtc_set_rate(const int32_t int_rate){
  
  return -1;
}
int32_t rtc_open(const uint8_t* filename){
  return -1;
}
int32_t rtc_close(int32_t fd){
  return -1;
}
*/
//-------------------------End Sandwich Part-----------------------------------







