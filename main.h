#pragma once

#include "port_expander.h"
#include "state_machine.h"
#include <avr/io.h>

/* AVR definitions*/
#define FOSC 16000000
#define I2C_FREQ 40000
#define SECOND 61

/* Scales to a standard of 1 sec, so scaler 0.5 would be half second
 * Be careful because the underlying value is 16 bits */
void configure_clock1(const int);

void configure_int0();
void configure_int1();
char check_hazard();
void increment_state();

// Tone Functions
void tone_controller();


// LED Functions
void led_controller();
void state_flash(sector_state * sector);
void sense_flash(sector_state * sector);
void isolate_flash(sector_state * sector);
void evac_flash(sector_state * sector);

// Sector Functions
void sector_controller();
char get_state_int(sector_state * sector);
char int_counter();
void reset_sector(sector_state * sector, char level);
void isolate_sector(sector_state * sector);

void transfer_state();

// State triggers
void trigger_isolate();
void trigger_emergency();
void trigger_evac();
void trigger_alert();
void trigger_reset();
void trigger_sector_evac();
void trigger_sensor();

// Display functions
void init_display();
void display_state();

void check_analog();
void set_next_state();
void ERROR();
char min(char, char);
void refresh_state();
const char sector_map(sector_states state);
const char tone_map(tone_states state);
const char reset_map(reset_states state);

void configure_int0(){
  /* EIMSK - External Interrupt Mask
   * Bit 7-2: Nothing
   * Bit 1: INT1
   * Bit 0: INT0
   */
  EIMSK |= 0b00000001;

  /* EICRA - External Int Control Reg
   * [-][-][-][-][ISC11][ISC10][ISC01][ISC00]
   * ISC1(1-0) - INT1
   * ISC0(1-0) - INT0
   * ISCx: 00 low level generate int req
   * ISCx: 01 logical change generates
   * ISCx: 10 Falling edge
   * ISCx: 11 Rising edge
   */
  EICRA |= 0b00000010;
};

void configure_int1(){
  /* EIMSK - External Interrupt Mask
   * Bit 7-2: Nothing
   * Bit 1: INT1
   * Bit 0: INT0
   */
  EIMSK |= 0b00000010;

  /* EICRA - External Int Control Reg
   * [-][-][-][-][ISC11][ISC10][ISC01][ISC00]
   * ISC1(1-0) - INT1
   * ISC0(1-0) - INT0
   * ISCx: 00 low level generate int req
   * ISCx: 01 logical change generates
   * ISCx: 10 Falling edge
   * ISCx: 11 Rising edge
   */
  EICRA |= 0b00001000;
};

void configure_int_d(){
	/* PCICR - Pin Change Interrupt Control Reg
	 * [-][-][-][-][-][PCIE2][PCIE1][PCIE0]
	 * PCIE0: Interrupts for port B
	 * PCIE1: Interrupts for port C
	 * PCIE2: Interrupts for port D
	 */
	PCICR = (1 << PCIE2);
	/* PCMSK - Pin Change Mask
	 * Which pins you want to trigger an interrupt
	 */
	PCMSK2 = (1 << PD5) | (1 << PD6) | (1 << PD7);
};

unsigned char swap(unsigned char x){
  return ((x & 0x0F)<<4 | (x & 0xF0)>>4);
};

void configure_clock1(const int freq){

  /* TIMSK1
   * Timer/Counter 1 Interrupt Mask Register
   * Bit 5: ICF1 - Input Capture Flag
   * Bit 2: OCF1B - Output Compare B Match
   * Bit 1: OCF1A -
   * bit 0: TOV1 - Overflow Flag
   */
  //TIMSK1 = 0b00000100;

  /* TCCR1B - Timer 1 Control Reg B
   * [ICNC1][ICES1][-][WGM13][WGM12][CS12][CS11][CS10]
   * ICNC1 - Input Capture Noise Canceler
   * ICES1 - Input Capture Edge Select
   * WGM - Waveform Gen Mode - Setting PWM/Normal
   * CS - Clock Select - Pre-scaling */
  //TCCR1B = 0b00000000; /* Stopping the clock */

  /* TCCR1A - Timer 1 Control Reg A
   * [COM1A1][COM1A0][COM1B1][COM1B0][-][-][WGM11][WGM10]
   * COM1A - Compare Output mode channel A
   * COM1B - ** for A
   * WGM - Wave Gen Mode */
  TCCR1A = 0b01000011;

  /* To Set WGM = 0000 and Prescaler to 1024
   * TCCR1B = [00][-][00][101]
   * TCCR1C = [FOC1A][FOC1B][-][-][-][-][-][-]
   * FOC - Force Output Compare for A/B */
  TCCR1B = 0b00011010;

  TCCR1C = 0b00000000;

	/* OCR1B - (OCR1BH & OCR1BL) - 16 Bit
   * Output Compare Register B
   * Bit 15-0: Output to compare to timer
   * 16,000,000/1024 = 15625 Ticks per sec */
  OCR1A = freq;

  /* TIFRx - Timer Interrupt Flag Registers
   * [-][-][-][-][-][OCFxB][OCFxA][TOVx]
   * OCFxB - Compare Match B
   * OCFxA - Compare Match A
   * TOVx - Timer Overflow flagc */
  //TIFR1 = 0b00000000;

  /* TCNT1 - (TCNT1H & TCNT1L) - 16 Bit
   * Timer/Counter 1
   * Bit 15-0: The value of the timer */
  TCNT1 = 0;

};

void configure_clock2(){
	TIMSK2 = 0b00000100; //Setting flag on compare b
	/* TCCR2B Timer/Counter Control Reg B
	 * [FOC2A][FOC2B][-][-][WGM22][CS22][CS21][CS20]
	 */
	TCCR2B = 0; // turning clock off

	/* TCCR2A Timer/Counter Control Reg A
	 * [COM2A1][COM2A0][COM2B1][COM2B0][-][-][WGM21][WGM20]
	 */
	TCCR2A = 0; // Setting waveform to normal op

	OCR2B = 255; // Setting max compare

	TIFR2 = 0b00000100; // ISR on compare match b

	/*
	 * (16,000,000/1024)/255 = 0.01632
	 * 1/0.01632 = 61 -> need 61 for 1 second
	 */
	TCCR2B = 0b00000111; // Setting prescaler to 1024

	TCNT2 = 0; // Setting clock to 0
}
