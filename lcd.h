#pragma once

/* LCD Definitions */
#define BACKLIGHT     0x08
#define DATA_BYTE     0x01
#define LCD_ADDRESS   0x27
#define LCD_CLEAR     0x01
#define LCD_POS       0x80
#define FOUR_BIT_MODE 0x28

/* LCD POSITIONS */
#define A 0
#define B 1
#define C 2
#define D 3
#define E 4
#define F 5
#define G 6

unsigned char lcd_position(unsigned char);
unsigned char lcd_init();
unsigned char lcd_write_str(char*, unsigned char, unsigned char);
