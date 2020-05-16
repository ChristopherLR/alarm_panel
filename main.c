#include <avr/io.h>
#include <avr/interrupt.h>
#include "main.h"
#include "port_expander.h"
#include "analog.h"
#include "lcd.h"
#include "twi.h"
#include "spi.h"
#include "state_machine.h"

/* Global variables */
float timer_scaler = 0.1;
unsigned char LED_VAL= 0xFF;
unsigned char TICK = 1;
char CHECK_PE = 0;
unsigned char SYSTEM_COUNTER = 2;
unsigned char INT_HELD = 0;
unsigned char INT_COUNTER = 1;
unsigned char LED_DISP1[] = "                ";
unsigned char LED_DISP2[] = "                ";

port_expander PE = { 0b00000101, 0b00000000, 0 };

reset_state reset = { reset_nominal };
tone_state tone = { tone_nominal };
sector_state sector_1 = { sensor_nominal };
sector_state sector_2 = { sensor_nominal };
sector_state sector_3 = { sensor_nominal };
major_state system_state = { &tone, &reset, &sector_1, &sector_2, &sector_3 };

int main() {

	DDRC  |= 0b00001110;
	PORTC |= 0b00000000;

	DDRB  |= 0b00000010;
	PORTB &= 0b11111101;

	configure_clock2();
	configure_int_d();
  configure_int0();
  configure_spi();
  analog_init();
  configure_port_expander();
  twi_init();
	sei();

  /* Clear any interrupts that may have occurred */
  spi_read(GPIOA, 0x00);
  spi_read(GPIOB, 0x00);
  unsigned char state = SUCCESS;

  state = lcd_init();

  spi_send(OLATA, PE.out_a);
  spi_send(OLATB, PE.out_b);

  /* Enable Global interrupts */
  sei();

  state = lcd_write_str("Hello World", 0x40, 11);

  if(state != SUCCESS) ERROR();

  /* Setting Port D to inputs */
  DDRD  &= 0b00000000;
  PORTD |= 0b11111111;

	TICK = 0;

  while (1) {
    if(TICK >= 61){
			SYSTEM_COUNTER ++;
			TICK = 0;
			PORTC ^= 0b00000010;

			if(INT_HELD){
				INT_COUNTER++;
			}
			if((PIND & 0b10000000) && (PIND & 0b01000000) && (PIND & 0b00100000)){
				INT_HELD = 0;
				INT_COUNTER = 0;
			}
			display_state();
		}
		if(CHECK_PE > 0){
			read_port_expander(&PE);
			CHECK_PE --;
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

ISR(PCINT2_vect){
	if(!(PIND & 0b10000000)){
		/* PORTB |= 0b00000010; */
		/* configure_clock1(2000); */
		INT_HELD = 1;
		sector_1.state = alert_sense;
		return;
	} else if(!(PIND & 0b01000000)){
		/* PORTB |= 0b00000010; */
		/* configure_clock1(200); */
		sector_2.state = alert_sense;
		INT_HELD = 1;
		return;
	} else if(!(PIND & 0b00100000)){
		/* PORTB |= 0b00000010; */
		/* configure_clock1(370); */
		INT_HELD = 1;
		sector_3.state = alert_sense;
		return;
	}
	INT_HELD = 0;
};

ISR(TIMER2_COMPB_vect){
	TCNT2 = 0;
	TICK++;
};

void display_state(){
	// Setting the display for the sector states
	LED_DISP1[0] = 'S';
	LED_DISP1[1] = '1';
	LED_DISP1[2] = ':';
	LED_DISP1[3] = sector_map(sector_1.state)[0];
	LED_DISP1[4] = sector_map(sector_1.state)[1];
	LED_DISP1[5] = 'S';
	LED_DISP1[6] = '2';
	LED_DISP1[7] = ':';
	LED_DISP1[8] = sector_map(sector_2.state)[0];
	LED_DISP1[9] = sector_map(sector_2.state)[1];
	LED_DISP1[10] = 'S';
	LED_DISP1[11] = '3';
	LED_DISP1[12] = ':';
	LED_DISP1[13] = sector_map(sector_3.state)[0];
	LED_DISP1[14] = sector_map(sector_3.state)[1];


	// Setting the display for the tone
	LED_DISP2[0] = 'R';
	LED_DISP2[1] = ':';
	LED_DISP2[2] = reset_map(reset.state);
	LED_DISP2[3] = ' ';
	LED_DISP2[4] = 'T';
	LED_DISP2[5] = ':';
	LED_DISP2[6] = tone_map(tone.state);

	convert_counters();

	lcd_write_str(LED_DISP1, 0x00, 16);
  lcd_write_str(LED_DISP2, 0x40, 16);
};


void refresh_state(){
};

const char* sector_map( sector_states state ){
	char *retc = "  ";
	switch (state) {
	case sensor_nominal:
		retc[0] = 'N';
		break;
	case alert_sense:
		retc[0] = 'A';
		break;
	case alert_trigger:
		retc[0] = 'T';
		break;
	case alert_evac:
		retc[0] = 'E';
		break;
	case alert_isolate:
		retc[1] = 'I';
		break;
	default:
		retc[0] = 'X';
		break;
	}
	return retc;
};

const char tone_map( tone_states state ){
	switch (state) {
	case tone_nominal:
		return 'N';
	case tone_alert:
		return 'A';
	case tone_evac:
		return 'E';
	default:
		return ' ';
	}
	return 'X';
};

const char reset_map( reset_states state ){
	switch (state) {
	case reset_nominal:
		return 'N';
	case reset_1:
		return '1';
	case reset_2:
		return '2';
	default:
		return ' ';
	}
	return 'X';
};


char min(char a, char b){
  if ( a < b ) return a;
  return b;
}



void convert_counters(){

	LED_DISP2[9] =  '0' + (SYSTEM_COUNTER/100)%10;
	LED_DISP2[10] = '0' + (SYSTEM_COUNTER/10)%10;
	LED_DISP2[11] = '0' + (SYSTEM_COUNTER)%10;

	LED_DISP2[13] = '0' + (INT_COUNTER/100)%10;
	LED_DISP2[14] = '0' + (INT_COUNTER/10)%10;
	LED_DISP2[15] = '0' + INT_COUNTER%10;
};

void increment_state(){
};

void transfer_state(){
};
void ERROR(){
};
