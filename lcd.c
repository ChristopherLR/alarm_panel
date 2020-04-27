#include "lcd.h"
#include <avr/io.h>
#include "twi.h"

unsigned char lcd_init(){

  unsigned char state = SUCCESS;

  twi_start();
  twi_send_addr(LCD_ADDRESS);
  twi_send_nibble(0x00);
  twi_send_nibble(0x30);
  twi_send_nibble(0x30);
  twi_send_nibble(0x30);
  twi_send_nibble(0x30);
  twi_send_nibble(0x20);
  twi_send_byte(0x0C, 0x00);
  twi_send_byte(0x01, 0x00);
  twi_stop();

  twi_start();
  twi_send_addr(LCD_ADDRESS);
  state = twi_send_byte(0x80, BACKLIGHT);
  twi_stop();
  unsigned char* init_str_1 = "LCD INIT";
  twi_frame twi_init;
  twi_init.address = LCD_ADDRESS;
  twi_init.num_bytes = 8;
  twi_init.lower = (BACKLIGHT | DATA_BYTE);
  twi_init.data = init_str_1;
  state = twi_send_data(twi_init);


  return state;
};

unsigned char lcd_position(unsigned char pos){
  unsigned char state;

  twi_start();
  twi_send_addr(LCD_ADDRESS);
  state = twi_send_byte(pos | 0x80, BACKLIGHT);
  twi_stop();


  return state;
};

unsigned char lcd_write_str(char* str,
                            unsigned char pos,
                            unsigned char size){
  unsigned char state = SUCCESS;
  twi_frame twi_frame;

  state = lcd_position(pos);

  twi_frame.address = LCD_ADDRESS;
  twi_frame.lower = (BACKLIGHT | DATA_BYTE);
  twi_frame.num_bytes = size;
  twi_frame.data = str;

  state = twi_send_data(twi_frame);

  return state;
};
