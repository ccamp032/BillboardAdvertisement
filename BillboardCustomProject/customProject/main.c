#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>
#include "nokia5110.c"
#include "advertisementBmpHex.h"
#include "logoBmpHex.h"
#include "timer.h"
#include "usart.h"
#include "shiftRegister.h"
#include <stdio.h>
#include <avr/eeprom.h>

/// variables ////
#define button1 (~PINC & 0x01)
#define button2 (~PINC & 0x02)
#define button3 (~PINC & 0x04)
unsigned char i,n,k,l,m; 
unsigned short data = 0x0000;
unsigned char FrameRate = 1000;
unsigned char receiveData = 0x00;

enum States {start, init, playdefault,playAdv,playLogo,playGif} state;
	
void Tick()
{
	switch(state)
	{
		case start:
			state = init;
			n = 0;
			i = 0;
			k = 0;
			l = 0;
			m = 0;
			break;
		case init:
		if(!button1 && !button2 && !button3 )
			   {
				   state = playdefault;
			   }
	    else if(button1 && !button2 && !button3 )
			   {
				   state = playAdv;
			   }
	    else if(button2 && !button1 && !button3)
			   {
				   state = playLogo;
			   }
	    else if(button3 && !button1 && !button2 )
			   {
				   
				   state = playGif;
			   }
			   break;
		case playdefault:
		receiveData = USART_Receive(0);
		 if(USART_HasTransmitted(0)) 
		 {
			 state = playdefault;
		 }
		    else if(!button1 && !button2 && !button3 )
			{
				state = playdefault;
			}
			else if(button1 && !button2 && !button3 )
			{
				state = playAdv;
			}
			else if(button2 && !button1 && !button3 )
			{
				state = playLogo;
			}
			else if(button3 && !button1 && !button2 )
			{
				
				state = playGif;
			}
			
		 
			break;
		case playAdv:
			if(button1)
			{
				state = playdefault;
			}
			else if(!button1 && !button2 && !button3 )
			{
				state = playAdv;
			}
			else if(button1 && !button2 && !button3 )
			{
				state = playAdv;
			}
			else if(button2 && !button1 && !button3)
			{
				state = playLogo;
			}
			else if(button3 && !button1 && !button2 )
			{
				
				state = playGif;
			}
			break;
		case playLogo:
			if(button2)
			{
				state = playdefault;
			}
			else if(!button2 && !button1 && !button3 )
			{
				state = playLogo;
			}
			else if(button1 && !button2 && !button3 )
			{
			state = playAdv;
			}
			else if(button3 && !button2 && !button1 )
			{
			state = playGif;
			}
			break;
		case playGif:
		   if(button3)
		   {
			   state = playdefault;
		   }
		   else if(!button3 && !button1 && !button2 )
		   {
			   state = playGif;
		   }
		   	else if(button1 && !button3 && !button2 )
		   	{
			   	state = playAdv;
		   	}
		   	else if(button2 && !button3 && !button2)
		   	{
			   	state = playLogo;
		   	}
			break;
		default:
			state = start;
			break;
	}
	
		switch(state)
		{
			case start:
				break;
			case init:
			nokia_lcd_clear();
			nokia_lcd_Default();
				break;
			case playdefault:
				nokia_lcd_clear();
				nokia_lcd_Default();
				USART_Send(0b00000011,0);
				data = sequence1[i];
				transmit_data(data);
				i++;
				break;
			case playAdv:
				nokia_lcd_clear();
				displayAdvertisement();
				data = sequence2[n];
				transmit_data(data);
				n++;
				break;
			case playLogo:
				nokia_lcd_clear();
				displayLogo();
				//k = 0;
				data = sequence3[k];
				transmit_data(data);
				k++;
				break;
			case playGif:
				nokia_lcd_clear();
				displayGifs();
				data = sequence4[l];
				transmit_data(data);
				l++;
				break;
			default:
				nokia_lcd_clear();
				state = start;
				break;
		}
}
////////////////////////////////////////////

int main(void)
{	
	DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0x00; PORTC = 0xFF; // Initialize DDRC to inputs
	DDRD = 0xFF; PORTD = 0x00;
	
	//communication
	initUSART(0);
	USART_Flush((0));
	initUSART(1);
	USART_Flush(1);
	
	TimerSet(25);
	TimerOn();
	
	nokia_lcd_init();
	nokia_lcd_clear();
	state = start;

	while (1)
	{
		Tick();
		while(!TimerFlag){}
		TimerFlag = 0;
	}
	
}

/*	
int main(void) 
{
		initUSART(0);
		USART_Flush((0));
		
		initUSART(1);
		USART_Flush(1);
	//	DDRA = 0x00; PORTA = 0xFF;
		//DDRB = 0xFF; PORTB = 0x00;
		DDRC= 0x00; PORTC = 0xFF;
		
		while(1) 
		{
			unsigned char button = (~PINC & 0x01);
			if(button) 
			{
				PORTC = 0x01;
				USART_Send(0b11110000,0);
				} 
				else 
				{
				PORTC = 0x00;
				USART_Send(0b00000000,0);
			}
		}
		
	} 
	
	
	*/

 	