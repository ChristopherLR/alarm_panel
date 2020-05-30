#include "analog.h"
#include <avr/io.h>


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
   * ADPS[2:0] = Prescaler Select Bits - Slows down the conversion rate
   */
	ADCSRA = 0;
	ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) ;
  //ADCSRA = 0b10101111;

  return 1;
}

unsigned char analog_read(){
  ADCSRA |= (1 << ADSC);
  while(ADCSRA & (1 << ADSC));

  return ADCH;
}
