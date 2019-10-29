/*
 * Blake Berry's Shift Register library
 * Email: error.brb@gmail.com
 */ 


#ifndef SHIFTREGISTER_H_
#define SHIFTREGISTER_H_

//shift register prototype

#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer.h"


const unsigned short sequence1[] = {0x8000,0x4000,0x2000,0x1000,0x0800,0x0400,0x0200,0x0100,0x0080,0x0040,0x0020,0x0010,0x0008,0x0004,0x0002,0x0001};
const unsigned short sequence2[] = {0x8000,0x7000,0x6000,0x8000,0x0800,0x0800,0x0700,0x0100,0x0010,0x0020,0x0020,0x0010,0x0001,0x0002,0x0002,0x0001};
const unsigned short sequence3[] = {0x8000,0x8001,0x8080,0x7008,0x0808,0x0800,0x8080,0x0008,0x8000,0x0100,0x1010,0x0101,0x1010,0x0011,0x0010,0x1001};
const unsigned short sequence4[] = {0x8080,0x8088,0x8080,0x8808,0x0808,0x0800,0x8080,0x0008,0x8888,0x0100,0x1010,0x0101,0x1010,0x0011,0x0010,0x1001};
const unsigned short sequence5[] = {0x8080,0x8088,0x8080,0x8808,0x0808,0x0808,0x8080,0x0008,0x8888,0x0101,0x1010,0x0101,0x1010,0x1011,0x0010,0x1001};

unsigned char j;
void transmit_data(unsigned short data) 
{
	int i;
	for (i = 0; i < 16 ; ++i) 
	{
		// Sets SRCLR to 1 allowing data to be set
		// Also clears SRCLK in preparation of sending data
		PORTD = 0x18;
		// set SER = next bit of data to be sent.
		PORTD |= ((data >> i) & 0x01);
		// set SRCLK = 1. Shifts next bit of data into the shift register
	PORTD |= 0x02;       
	}
	// set RCLK = 1. Copies data from “Shift” register to “Storage” register
	PORTD |= 0x04;
	// clears all lines for a new transmission
	PORTD = 0x00;
	
}
/*
void transmit_data(unsigned short data) {
	int i;
	for (i = 0; i < 16 ; ++i) {
		// Sets SRCLR to 1 allowing data to be set
		// Also clears SRCLK in preparation of sending data
		PORTB = 0x18;
		// set SER = next bit of data to be sent.
		PORTB |= ((data >> i) & 0x01);
		// set SRCLK = 1. Shifts next bit of data into the shift register
	PORTB |= 0x02;       }
	// set RCLK = 1. Copies data from “Shift” register to “Storage” register
	PORTB |= 0x04;
	// clears all lines for a new transmission
	PORTB = 0x00;
}
*/

#endif /* SHIFTREGISTER_H_ */