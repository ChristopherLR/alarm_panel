#pragma once

//#define TWI_BIT_RATE ((FOSC/I2C_FREQ)-16)/2
#define TWI_BIT_RATE 193
/* TWI Definitions */
#define OWN_ADR       00
#define SUCCESS       0x53
#define FAIL          0x46
#define START         0x10
#define ADR_ACK       0x18
#define DATA_ACK      0x28
/* Encapsulating the twi transmission into a struct */
typedef struct {
  unsigned char address;    // Address of slave
  unsigned char num_bytes;  // Number of bytes in data
  unsigned char lower;      // Setting the Backlight/Data
  unsigned char *data;      // Pointer to the data for transmission
} twi_frame;

char twi_init();
unsigned char twi_start();
void twi_wait();
void twi_stop();
unsigned char twi_send_addr(unsigned char);
unsigned char twi_send_byte(unsigned char, unsigned char);
unsigned char twi_send_nibble(unsigned char);
unsigned char twi_send_data(twi_frame);


