#include "port_expander.h"
#include <avr/io.h>
#include "spi.h"


void configure_port_expander(){
  /* IOCON */
  spi_send(IOCON, 0b01000000);
  /* IODIRx Port x DDR */
  spi_send(IODIRB, 0b00000000);
  spi_send(IODIRA, 0b01110000);
  /* GPPUx Port x pullups */
  spi_send(GPPUB, 0b00000000);
  spi_send(GPPUA, 0b01110000);
  /* GPINTENx Interrupt on change */
  spi_send(GPINTENB, 0b00000000);
  spi_send(GPINTENA, 0b01110000);
  /* INTCONx Compare DEFVAL=1 or Prev-Val=0 */
  spi_send(INTCONB, 0b00000000);
  spi_send(INTCONA, 0b01110000);
  /* DEFVAL - sets the default val */
  spi_send(DEFVALB, 0b00000000);
  spi_send(DEFVALA, 0b01110000);

};

void read_port_expander(port_expander * PE){
  char tmp = 0;
  tmp = spi_read(GPIOA, 0x00);
  /* Checking F & D */
  if(!(tmp & 0b10000000)) PE->in |= 0b000000001;
  if(!(tmp & 0b00001000)) PE->in |= 0b000000010;

  tmp = spi_read(GPIOB, 0x00);
  /* Checking G & B */
  if(!(tmp & 0b10000000)) PE->in |= 0b000000100;
  if(!(tmp & 0b00001000)) PE->in |= 0b000001000;
}
