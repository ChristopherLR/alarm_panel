#include "analog.h"
#include <avr/io.h>

void check_analog(float * timer_scaler){
  unsigned char an = analog_read();
  float scaled = 0;
  if(an < 20 ){
    scaled = 0;
  } else if(an < 50){
    scaled = 0.2;
  } else if(an < 120){
    scaled = 0.4;
  } else if( an < 170){
    scaled = 0.6;
  } else if( an < 220){
    scaled = 0.8;
  } else scaled = 0.9;
  *timer_scaler = 1.0 - scaled;
};

char analog_init(){
  DDRC &= 0b11111110;

  /* ADMUX - ADC Mulitplexer Selection
   * [REFS1][REFS0][ADLAR][-][MUX3][MUX2][MUX1][MUX0]
   */
  ADMUX = 0b01100000;

  /* ADCSRA - ADC Control and Status Reg
   * [ADEN][ADSC][ADATE][ADIF][ADIE][ADPS2][ADPS1][ADPS0]
   * ADEN = Interrupt Enable
   * ADSC = ADC Start Conversion
   * ADATE = ADC Auto Trigger Enable
   * ADIF = ADC Interrupt Flag
   * ADIE = ADC Interrupt Enable
   * ADPS = Prescaler Select Bits - Slows down the conversion rate
   */
  ADCSRA = 0b10000111;

  return 1;
}

unsigned char analog_read(){
  ADCSRA |= (1 << ADSC);
  while(ADCSRA & (1 << ADSC));

  return ADCH;
}
