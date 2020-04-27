#include "spi.h"
#include <avr/io.h>


void configure_spi(){
  /* DDRB - Data Direction B
   * [XTAL][XTAL][SCK][MISO0][MOSI0][!SS][-][-]
   */
  DDRB = 0b00101111;
  PORTB |= 0b00000101; /* Setting Slaves back high */

  /* SPCR - SPI Control Register
   * [SPIE][SPE][DORD][MSTR][CPOL][CPHA][SPR1][SPR0]
   * SPIE - Interrupt Enable
   * SPE - SPI Enable
   * DORD - Data Order 1: LSB, 0: MSB
   * MSTR - Master/Slave Select
   * SPR - Clock Rate Select (used with SPI2X)
   */
  SPCR |= (1 << SPE) | (1 << MSTR);

  /* SPSR - SPI Status Register
   * [SPIF][WCOL][-][-][-][-][-][SPI2X]
   */
  SPSR = 0;
};


char spi_master_transmit(char data){
  /* SPDR - SPI Data Register */
  SPDR = data;
  /* Wait for transfer */
  while(!(SPSR & (1 << SPIF)));
  return SPDR;
};

char spi_send(char cmd, char data){

  char retv = 0;

	/* Clear Bit 2 (SS) */
	PORTB &= 0b11111011;

	spi_master_transmit(0x40);
  spi_master_transmit(cmd);
  retv = spi_master_transmit(data);

	/* Set   Bit 2 (SS) */
	PORTB |= 0b00000100;

  return retv;
};

char spi_read(char cmd, char data){

  char retv = 0;

	/* Clear Bit 2 (SS) */
	PORTB &= 0b11111011;

  spi_master_transmit(0x41);
  spi_master_transmit(cmd);
  retv = spi_master_transmit(data);

	/* Set   Bit 2 (SS) */
	PORTB |= 0b00000100;

  return retv;
};
