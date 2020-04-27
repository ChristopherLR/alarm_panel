#include <avr/io.h>
#include <avr/interrupt.h>
#include "main.h"
#include "port_expander.h"
#include "lcd.h"
#include "twi.h"
#include "spi.h"

/* Global variables */
float timer_scaler = 0.1;
unsigned char LED_VAL= 0xFF;
unsigned char TICK = 1;
char CHECK_PE = 0;
unsigned char LED_DISP1[] = "                ";
unsigned char LED_DISP2[] = "                ";

port_expander PE = {0b00000111, 0b00000000, 0};


int main() {

	DDRC  |= 0b00001110;
	PORTC |= 0b00000000;

	DDRB  |= 0b00000010;
	PORTB &= 0b11111101;

	configure_clock2();
  configure_int0();
  configure_spi();
  analog_init();
  configure_port_expander();
  twi_init();

  /* Clear any interrupts that may have occurred */
  spi_read(GPIOA, 0x00);
  spi_read(GPIOB, 0x00);
  unsigned char state = SUCCESS;

  state = lcd_init();

  spi_send(OLATA, 0xFF);
  spi_send(OLATB, 0xFF);

  /* Enable Global interrupts */
  sei();

  unsigned char str[] = "HELLO WORLD";
  state = lcd_write_str(str, 0x40, 11);

  if(state != SUCCESS) ERROR();

  /* Setting Port D to inputs */
  DDRD  &= 0b00000000;
  PORTD |= 0b11111111;

	unsigned char freq = 0;
	TICK = 0;

  while (1) {
    if(TICK >= 61){
			TICK = 0;
			PORTC ^= 0b00000010;
			freq ++;
			display_counter((freq/100)%10, (freq/10)%10, freq%10);
		}

    if(CHECK_PE > 0){
        read_port_expander(&PE);
        CHECK_PE --;
		}
		if(!(PIND & 0b10000000)){
			/* PORTB |= 0b00000010; */
			configure_clock1(2000);
		} else if(!(PIND & 0b01000000)){
			/* PORTB |= 0b00000010; */
			configure_clock1(2500);
		} else if(!(PIND & 0b00100000)){
			/* PORTB |= 0b00000010; */
			configure_clock1(3000);
		}
	}
  return 0;
};

/* PE_A */
ISR(INT0_vect){
  CHECK_PE++;
  //read_port_expander_a();
	return;
};

ISR(TIMER1_COMPB_vect){
   /* TCNT1 - (TCNT1H & TCNT1L) - 16 Bit
   * Timer/Counter 1
   * Bit 15-0: The value of the timer
   */
  TCNT1 = 0;
};

ISR(TIMER2_COMPB_vect){
	TCNT2 = 0;
	TICK++;
};

void display_state(){
};

void refresh_state(){
};


char min(char a, char b){
  if ( a < b ) return a;
  return b;
}


void display_counter(unsigned char h, unsigned char t, unsigned char u){
	unsigned char hundreds = h;
  unsigned char tens = t;
  unsigned char units = u;

  LED_DISP1[13] = '0' + hundreds;
  LED_DISP1[14] = '0' + tens;
  LED_DISP1[15] = '0' + units;

  lcd_write_str(LED_DISP1, 0x00, 16);

};

void increment_state(){
};

void transfer_state(){
};
void ERROR(){
};
