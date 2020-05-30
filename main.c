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
char CHECK_PE = 0;
unsigned char SYSTEM_TICK = 0;
unsigned char SYSTEM_TIMER = 0;
unsigned char LED_TOGGLE = 0;
unsigned char ISO_TOGGLE = 0;
unsigned char INT_TICK = 0;
unsigned char INT_COUNTER = 0;
unsigned char INT_TIMER = 0;
unsigned char RESET_TIMER = 0;
unsigned char RESET_TICK = 0;
unsigned char RESET_COUNTER = 0;
unsigned char TONE_TICK = 0;
unsigned char TONE_COUNTER = 0;
unsigned char TONE_LATCH = 0;
unsigned char SECTOR_LATCH = 0;
unsigned char DISPLAY_INTERVAL = 30;
unsigned char PORT_EXPANDER_INTERVAL = 30;
unsigned char PROCESS_OFFSET = 15;
unsigned char LED_DISP1[] = "                ";
unsigned char LED_DISP2[] = "                ";

port_expander PE = { 0b00000000, 0b00000000, 0 };

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

  init_display();
  state = lcd_write_str(LED_DISP1, 0x00, 16);
  if(state != SUCCESS) ERROR();
  state = lcd_write_str(LED_DISP2, 0x40, 16);
  if(state != SUCCESS) ERROR();

  /* Setting Port D to inputs */
  DDRD  &= 0b00000000;
  PORTD |= 0b11111111;

	SYSTEM_TICK = 0;
	INT_TICK = 0;

  while (1) {
    // Second elapsed
    if (SYSTEM_TICK >= SECOND){
			SYSTEM_TICK = 0;
			SYSTEM_TIMER ++;
		};

    // Counter For how long the sensor activated
		if (INT_TICK >= SECOND ){
			INT_TICK = 0;
      INT_TIMER++;
		};

    if (RESET_TICK >= SECOND * 2){
      RESET_TICK = 0;
      RESET_TIMER = 1;
    }

    // If the PE interrupt has been triggered
		if (CHECK_PE > 0){
			read_port_expander(&PE);
      if(PE.in & 0b00000001){
        trigger_isolate();
      } else if (PE.in & 0b00000010){
        trigger_reset();
      } else if (PE.in & 0b00000100){
        trigger_emergency();
      };
			CHECK_PE --;
		};

    // Processing the display
    if ((SYSTEM_TICK+PROCESS_OFFSET)%DISPLAY_INTERVAL == 0){
      display_state();
    };

    // Processing the Port Expander with offset
    if (SYSTEM_TICK%PORT_EXPANDER_INTERVAL == 0){
      spi_send(OLATA, PE.out_a);
    };

    // Moving the program along
		check_analog();
		increment_state();
	}
  return 0;
};

ISR(INT0_vect){
  // Read port expander on next main loop
  CHECK_PE++;
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
		trigger_sensor(&sector_1);
	}
	if(!(PIND & 0b01000000)){
		trigger_sensor(&sector_2);
	}
	if(!(PIND & 0b00100000)){
		trigger_sensor(&sector_3);
	}
};

ISR(TIMER2_COMPB_vect){
	TCNT2 = 0;
	SYSTEM_TICK++;
	if(int_counter() > 0) INT_TICK++;
  if(reset.state != reset_nominal ) RESET_TICK++;
};

void sense_flash(sector_state * sector){
  if (LED_TOGGLE){
    if (sector == &sector_1) {
      PE.out_a |= 0b00000001;
      PORTC    |= 0b00000010;
    };
    if (sector == &sector_2) {
      PE.out_a |= 0b00000010;
      PORTC    |= 0b00000100;
    };
    if (sector == &sector_3) {
      PE.out_a |= 0b00000100;
      PORTC    |= 0b00001000;
    };
  }else {
    if (sector == &sector_1) {
      PE.out_a &= 0b11111110;
      PORTC    &= 0b11111101;
    };
    if (sector == &sector_2) {
      PE.out_a &= 0b11111101;
      PORTC    &= 0b11111011;
    };
    if (sector == &sector_3) {
      PE.out_a &= 0b11111011;
      PORTC    &= 0b11110111;
    };
  };
};

// Set the relevant sector LEDs to solid
void evac_flash(sector_state * sector){
  if (sector == &sector_1) {
    PE.out_a |= 0b00000001;
    PORTC    |= 0b00000010;
  };
  if (sector == &sector_2) {
    PE.out_a |= 0b00000010;
    PORTC    |= 0b00000100;
  };
  if (sector == &sector_3) {
    PE.out_a |= 0b00000100;
    PORTC    |= 0b00001000;
  };
};

void nominal_clear(sector_state * sector){
  if (sector == &sector_1) {
    PE.out_a &= 0b11111110;
    PORTC    &= 0b11111101;
  };
  if (sector == &sector_2) {
    PE.out_a &= 0b11111101;
    PORTC    &= 0b11111011;
  };
  if (sector == &sector_3) {
    PE.out_a &= 0b11111011;
    PORTC    &= 0b11110111;
  };
}

// Set the relevant sector LEDs to toggle every 2 seconds
void isolate_flash(sector_state * sector){
  if (ISO_TOGGLE){
    if (sector == &sector_1) {
      PE.out_a |= 0b00000001;
      PORTC    |= 0b00000010;
    };
    if (sector == &sector_2) {
      PE.out_a |= 0b00000010;
      PORTC    |= 0b00000100;
    };
    if (sector == &sector_3) {
      PE.out_a |= 0b00000100;
      PORTC    |= 0b00001000;
    };
  }else {
    if (sector == &sector_1) {
      PE.out_a &= 0b11111110;
      PORTC    &= 0b11111101;
    };
    if (sector == &sector_2) {
      PE.out_a &= 0b11111101;
      PORTC    &= 0b11111011;
    };
    if (sector == &sector_3) {
      PE.out_a &= 0b11111011;
      PORTC    &= 0b11110111;
    };
  };
};

// Set the sector LED to the appropriate state
void state_flash(sector_state * sector){
  switch(sector->state){
  case sensor_nominal:
    nominal_clear(sector);
    break;
  case alert_sense:
    sense_flash(sector);
    break;
  case alert_trigger:
    sense_flash(sector);
    break;
  case alert_evac:
    evac_flash(sector);
    break;
  case alert_isolate:
    isolate_flash(sector);
    break;
  default:
    break;
  };
};

void led_controller(){
  LED_TOGGLE = SYSTEM_TICK < PORT_EXPANDER_INTERVAL ? 1 : 0;
  ISO_TOGGLE = (SYSTEM_TIMER%2 == 0) ? ISO_TOGGLE^1 : ISO_TOGGLE;
  state_flash(&sector_1);
  state_flash(&sector_2);
  state_flash(&sector_3);
};


char get_state_int(sector_state * sector){
  if (sector->state == alert_sense || sector->state == alert_trigger) return 1;
  return 0;
};

char int_counter(){
	char init = 0;
	if (get_state_int(&sector_1)) init++;
	if (get_state_int(&sector_2)) init++;
	if (get_state_int(&sector_3)) init++;
	return init;
};


void trigger_sector_evac(){
	if (get_state_int(&sector_1)) sector_1.state = alert_evac;
	if (get_state_int(&sector_2)) sector_2.state = alert_evac;
	if (get_state_int(&sector_3)) sector_3.state = alert_evac;
  tone.state = tone_evac;
  reset.state = reset_nominal;
};

void increment_state(){
  sector_controller();
  led_controller();

};

void sector_controller(){
  unsigned char count = int_counter();
  if (count >= 2 || INT_TIMER >= 6) {
    if (SECTOR_LATCH != 2){
      SECTOR_LATCH = 2;
      trigger_sector_evac();
    }
  } else if ( INT_TIMER >=3 && INT_TIMER < 6 && count == 1){
    if (SECTOR_LATCH != 1){
      SECTOR_LATCH = 1;
      trigger_alert();
    }
  } else {
    SECTOR_LATCH = 0;
  }
};


void trigger_sensor(sector_state * sector){
  if(sector->state == sensor_nominal){
    sector->state = alert_sense;
    sector_controller();
  }
};


void trigger_isolate(){
  LED_DISP1[15] = 'I';
};
void trigger_reset(){
  LED_DISP1[15] = 'R';
  tone.state = tone_nominal;
  if ( reset.state != reset_nominal && RESET_TIMER ){
    if ( reset.state == reset_1 ){
      reset.state = reset_2;
      reset_sector(&sector_1, 1);
      reset_sector(&sector_2, 1);
      reset_sector(&sector_3, 1);
      INT_TIMER = 0;
      RESET_TICK = 0;
      RESET_TIMER = 0;
    } else if ( reset.state == reset_2 ){
      reset.state = reset_nominal;
      reset_sector(&sector_1, 2);
      reset_sector(&sector_2, 2);
      reset_sector(&sector_3, 2);
      INT_TIMER = 0;
      RESET_TICK = 0;
      RESET_TIMER = 0;
    }
  } else if (reset.state == reset_nominal && ( RESET_TICK > 10 || RESET_TIMER)) {
    reset.state = reset_1;
    RESET_TICK = 0;
    RESET_TIMER = 0;
  }
};
void trigger_emergency(){
  LED_DISP1[15] = 'E';
  reset.state = reset_nominal;
};
void trigger_alert(){
  if (get_state_int(&sector_1)) sector_1.state = alert_trigger;
  if (get_state_int(&sector_2)) sector_2.state = alert_trigger;
  if (get_state_int(&sector_3)) sector_3.state = alert_trigger;
  if (tone.state == tone_nominal) tone.state = tone_alert;
  reset.state = reset_nominal;
};

void tone_controller(){
  if (tone.state == tone_nominal){
    if (TONE_LATCH != 1){
      TONE_LATCH = 1;
      TCCR1B = 0;
    }
    return;
  } else if (tone.state == tone_alert){
    if (TONE_LATCH != 2){
      TONE_LATCH = 2;
      configure_clock1(2000);
    }
  } else if (tone.state == tone_evac){
    if ( TONE_TICK > 2 * SECOND){
      if (TONE_LATCH != 3 ){
        TONE_LATCH = 3;
        configure_clock1(200);
      }
    } else {
      if (TONE_LATCH != 4){
        TONE_LATCH = 4;
        configure_clock1(370);
      }
    }
  }
};

// level is the decision between resetting isolate = 2 or standard = 1
void reset_sector(sector_state * sector, char level){
  sector_states state = sector->state;
  if(level == 1){
    if(state == alert_sense || state == alert_trigger || state == alert_evac ){
      sector->state = sensor_nominal;
    }
  } else if (level == 2){
    sector->state = sensor_nominal;
  }
};

void check_analog(){
	unsigned char ac = analog_read();

	if ( ac <= 175 && ac >= 165 ) {
		trigger_isolate();
		return;
	} else if ( ac <= 165 && ac >= 145){
		trigger_reset();
		return;
	} else if ( ac <= 100 && ac >= 80 ){
		trigger_emergency();
		return;
	}
};

// Setting up boilerplate LCD display
void init_display(){
  // ROW 1
  LED_DISP1[0] = 'S';
	LED_DISP1[1] = '1';
	LED_DISP1[2] = ':';
  LED_DISP1[3] = ' '; // S1 STATE
  LED_DISP1[4] = ' '; // ISOLATED?
	LED_DISP1[5] = 'S';
	LED_DISP1[6] = '2';
	LED_DISP1[7] = ':';
  LED_DISP1[8] = ' '; // S2 STATE
  LED_DISP1[9] = ' '; // ISOLATED?
  LED_DISP1[10] = 'S';
	LED_DISP1[11] = '3';
	LED_DISP1[12] = ':';
  LED_DISP1[13] = ' '; // S3 STATE
  LED_DISP1[14] = ' '; // ISOLATED?
  LED_DISP1[15] = ' '; // RESERVED
  // ROW 2
  LED_DISP2[0] = 'R';
	LED_DISP2[1] = ':';
  LED_DISP2[2] = ' '; // RESET STATE
  LED_DISP2[3] = ' ';
	LED_DISP2[4] = 'T';
	LED_DISP2[5] = ':';
  LED_DISP2[6] = ' '; // TONE STATE
  LED_DISP2[7] = ' ';
  LED_DISP2[8] = ' ';
  LED_DISP2[9] = '0'; // System Timer
  LED_DISP2[10] = '0'; // System Timer
  LED_DISP2[11] = '0'; // System Timer
  LED_DISP2[12] = ' ';
  LED_DISP2[13] = '0'; // RESET Timer
  LED_DISP2[14] = ' '; // RESERVED
  LED_DISP2[15] = '0'; // INT Timer
}

void display_state(){
	// Setting the display for the sector states
	LED_DISP1[3] = sector_map(sector_1.state)[0];
	LED_DISP1[4] = sector_map(sector_1.state)[1];

  LED_DISP1[8] = sector_map(sector_2.state)[0];
	LED_DISP1[9] = sector_map(sector_2.state)[1];

  LED_DISP1[13] = sector_map(sector_3.state)[0];
	LED_DISP1[14] = sector_map(sector_3.state)[1];

	lcd_write_str(LED_DISP1, 0x00, 16);

  // Setting the display for the reset and tone states
	LED_DISP2[2] = reset_map(reset.state);
	LED_DISP2[6] = tone_map(tone.state);

  // Setting Relevant Timers
  LED_DISP2[9] =  '0' + (SYSTEM_TIMER/100)%10;
	LED_DISP2[10] = '0' + (SYSTEM_TIMER/10)%10;
	LED_DISP2[11] = '0' + SYSTEM_TIMER%10;

  LED_DISP2[13] = '0' + RESET_TIMER%10;
	LED_DISP2[15] = '0' + INT_TIMER%10;

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
		retc[0] = 'S';
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



void transfer_state(){
};
void ERROR(){
};
