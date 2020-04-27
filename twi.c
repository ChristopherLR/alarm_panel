#include "twi.h"
#include <avr/io.h>

char twi_init(){

  DDRC &= 0b11001111;

  PORTC |= 0b00110000;

  /* TWAR - TWI Address */
  TWAR = OWN_ADR;

  /* TWBR - TWI Bit Rate Reg
   * SCL = Fosc / (16 + 2(TWBR).(TWPS))
   * We want SCL = 40KHz
   * Fosc = 16MHz
   * TWBR = 193 */
  TWBR = TWI_BIT_RATE;

  TWCR = (1 << TWEN);

  return 1;
};

unsigned char twi_start(){
  /* TWCR - TWI Control Register
   * [TWINT][TWEA][TWSTA][TWSTO][TWWC][TWEN][-][TWIE]
   * TWI Interrupt = 1 to clear flag, set when job is done
   * TWI Enable Ack = 1 ack pulse gen when conditions met
   * TWI STArt = 1 -> Generates start condition when avail
   * TWI STop = 1 -> Gen stop cond, cleared automaticall
   * TWI Write Collision = 1 when writing to TWDR and TWINT low
   * TWI Enable Bit = Enable TWI transmission
   * TWI Interrupt Enable = */
  TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);

  twi_wait();

  /* TWSR - TWI Status Register
   * [TWS7][TWS6][TWS5][TWS4][TWS3][-][TWPS1][TWPS0]
   * TWI Status = 5 Bit status of TWI
   * TWI PreScaler = sets the pre-scaler (1, 4, 16, 64) */
  if(TWSR != START) return FAIL;

  return SUCCESS;
};

void twi_stop(){
  /* Sending the stop condition */
  TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
};

unsigned char twi_send_data(twi_frame tx_frame){
  unsigned char state, i;
  state = SUCCESS;

  state = twi_start();
  if(state == SUCCESS){
  	state = twi_send_addr(tx_frame.address);
  } else {
  	twi_stop();
  	return state;
  }

  for(i = 0; ((i<tx_frame.num_bytes)&&(state=SUCCESS)); i++)
  			state = twi_send_byte(tx_frame.data[i],tx_frame.lower);

  twi_stop();

  return state;
};

unsigned char twi_send_byte(unsigned char data,
                            unsigned char lower){
  unsigned char state = SUCCESS;
  unsigned char first_nib  = (data & 0xF0) | lower;
  unsigned char second_nib = ((data << 4)&0xF0) | lower;

  state = twi_send_nibble(first_nib);
  if(state != SUCCESS) return state;
  state = twi_send_nibble(second_nib);
  if(state != SUCCESS) return state;

  return state;
};

unsigned char twi_send_nibble(unsigned char nibble){
  unsigned char tx = nibble ;
  TWDR = tx | 0x04;
  TWCR = (1 << TWINT) | (1 << TWEN); // Start transmission
  twi_wait();

  if((TWSR&0xF8) != DATA_ACK) return TWSR;

  TWDR = tx & 0xFB;
  TWCR = (1 << TWINT) | (1 << TWEN); // Start transmission
  twi_wait();

  if((TWSR&0xF8) != DATA_ACK) return TWSR;
  return SUCCESS;
};

unsigned char twi_send_addr(unsigned char addr){

  twi_wait();

  TWDR = addr + addr; //send Address across twi
  TWCR = (1 << TWINT) | (1 << TWEN); // Start transmission
  twi_wait();
  if((TWSR&0xF8) != ADR_ACK) return TWSR; // Checking for ack
  return SUCCESS;

};

void twi_wait(){
  while(!(TWCR & (1 << TWINT)));
};

