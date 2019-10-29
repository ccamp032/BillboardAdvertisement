/* Nokia 5110 LCD AVR Library
 *  Little buster's library
 *  https://github.com/LittleBuster
 */

#include "nokia5110.h"
#include "advertisementBmpHex.h"
#include "logoBmpHex.h"
#include "gif.h"
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <util/delay.h>
/*
Row Address (Y address): 
There are total 6 rows. Each row has height of 8 pixels. 
So row address given as Y address = 0 to 5.
Column Address (X Address): 
As display has total 84 pixels in each row, so X address = 0 to 83


As display has total pixels= 48x84 = 4032, We can also provide the address of the pixels from 0 to 4031.
*/

const unsigned char frameRate = 150;

static struct {
    /* screen byte massive */
    uint8_t screen[504]; //   [ (6 rows)  * (84 pixels in one row) ] =  504

    /* cursor position */
    uint8_t cursor_x;
    uint8_t cursor_y;

} nokia_lcd = {
    .cursor_x = 0,
    .cursor_y = 0
};

/**
 * Sending data to LCD
 * @bytes: data
 * @is_data: transfer mode: 1 - data; 0 - command;
 */
/************************************************************************/
/* 

Reference PORTS 
#define PORT_LCD PORTA
#define DDR_LCD DDRA

#define LCD_SCE PA1
#define LCD_RST PA0
#define LCD_DC PA2
#define LCD_DIN PA3
#define LCD_CLK PA4
#define LCD_CONTRAST 0x40
                                                                     */
/************************************************************************/


static void write(uint8_t bytes, uint8_t is_data)
{
	// PORT_LCD = PORTA 
	register uint8_t i;
	/* Enable controller and LCD_SCE = PA1 and SCE is Chip select*/
	PORT_LCD &= ~(1 << LCD_SCE);

	/* We are sending data and LCD_DC = data/command */
	if (is_data)
		PORT_LCD |= (1 << LCD_DC);
	/* We are sending commands */
	else
		PORT_LCD &= ~(1 << LCD_DC);

	/* Send bytes and data in is a Din pin is used for sending the data to the Nokia5110 display. */
	for (i = 0; i < 8; i++) 
	{
		/* Set data pin to byte state and LCD_DIN = PA3*/
		if ((bytes >> (7-i)) & 0x01)
			PORT_LCD |= (1 << LCD_DIN);
		else
			PORT_LCD &= ~(1 << LCD_DIN);
        /* Clk is the clock pin of Nokia5110. This is the SCK pin (Serial Clock) of SPI protocol*/
		/* Blink clock and LCD_CLK = PA4 */
		PORT_LCD |= (1 << LCD_CLK);
		PORT_LCD &= ~(1 << LCD_CLK);
	}

	/* Disable controller and LCD_SCE = PA1 */
	PORT_LCD |= (1 << LCD_SCE);
}
// DC , Data/Command 
static void write_cmd(uint8_t cmd)
{   // DC = 0, for sending commands to the display
	write(cmd, 0);
}

static void write_data(uint8_t data)
{ // DC = 1 , for sending data to the display 
	write(data, 1);
}

/*
 * Public functions
 */

void nokia_lcd_init(void)
{
	register unsigned i;
	/* Set pins as output and DDR_LCD = DDRA */
	DDR_LCD |= (1 << LCD_SCE); // SCE = PA1
	DDR_LCD |= (1 << LCD_RST); // RST = PA0
	DDR_LCD |= (1 << LCD_DC); //  DC =  PA2
	DDR_LCD |= (1 << LCD_DIN); // DIN = PA3
	DDR_LCD |= (1 << LCD_CLK); // CLK = PA4

	/* Reset display */
	PORT_LCD |= (1 << LCD_RST);
	PORT_LCD |= (1 << LCD_SCE);
	_delay_ms(10);
	PORT_LCD &= ~(1 << LCD_RST);
	_delay_ms(70);
	PORT_LCD |= (1 << LCD_RST);

	/*
	 * Initialize display
	 */
	/* Enable controller */
	PORT_LCD &= ~(1 << LCD_SCE);
	/* -LCD Extended Commands mode- */
	write_cmd(0x21);
	/* LCD bias mode 1:48 */
	write_cmd(0x13);
	/* Set temperature coefficient */
	write_cmd(0x06);
	/* Default VOP (3.06 + 66 * 0.06 = 7V) */
	write_cmd(0xC2);
	/* Standard Commands mode, powered down */
	write_cmd(0x20);
	/* LCD in normal mode */
	write_cmd(0x09);

	/* Clear LCD RAM */
	write_cmd(0x80);
	write_cmd(LCD_CONTRAST);
	for (i = 0; i < 504; i++)
		write_data(0x00);

	/* Activate LCD */
	write_cmd(0x08);
	write_cmd(0x0C);
}

void nokia_lcd_clear(void)
{
	register unsigned i;
	/* Set column and row to 0 */
	write_cmd(0x80);
	write_cmd(0x40);
	/*Cursor too */
	nokia_lcd.cursor_x = 0;
	nokia_lcd.cursor_y = 0;
	/* Clear everything (504 bytes = 84cols * 48 rows / 8 bits) */
	for(i = 0;i < 504; i++)
		nokia_lcd.screen[i] = 0x00;
}

void nokia_lcd_power(uint8_t on)
{
	write_cmd(on ? 0x20 : 0x24);
}

void nokia_lcd_set_pixel(uint8_t x, uint8_t y, uint8_t value)
{
	uint8_t *byte = &nokia_lcd.screen[y/8*84+x];
	if (value)
		*byte |= (1 << (y % 8));
	else
		*byte &= ~(1 << (y %8 ));
}

void nokia_lcd_write_char(char code, uint8_t scale)
{
	register uint8_t x, y;

	for (x = 0; x < 5*scale; x++)
		for (y = 0; y < 7*scale; y++)
			if (pgm_read_byte(&CHARSET[code-32][x/scale]) & (1 << y/scale))
				nokia_lcd_set_pixel(nokia_lcd.cursor_x + x, nokia_lcd.cursor_y + y, 1);
			else
				nokia_lcd_set_pixel(nokia_lcd.cursor_x + x, nokia_lcd.cursor_y + y, 0);

	nokia_lcd.cursor_x += 5*scale + 1;
	if (nokia_lcd.cursor_x >= 84) {
		nokia_lcd.cursor_x = 0;
		nokia_lcd.cursor_y += 7*scale + 1;
	}
	if (nokia_lcd.cursor_y >= 48) {
		nokia_lcd.cursor_x = 0;
		nokia_lcd.cursor_y = 0;
	}
}

void nokia_lcd_write_string(const char *str, uint8_t scale)
{
	while(*str)
		nokia_lcd_write_char(*str++, scale);
}

void nokia_lcd_set_cursor(uint8_t x, uint8_t y)
{
	nokia_lcd.cursor_x = x;
	nokia_lcd.cursor_y = y;
}

void nokia_lcd_render(void)
{
	register unsigned i;
	/* Set column and row to 0 */
	write_cmd(0x80);
	write_cmd(0x40);

	/* Write screen to display */
	for (i = 0; i < 504; i++)
		write_data(nokia_lcd.screen[i]);
}

void nokia_lcd_bitmap(void)
{
	register unsigned i;
	write_cmd(0x80);
	write_cmd(0x40);

	for (i = 0; i < 504; i++)
		write_data(pickYourPoison0[i]);
		_delay_ms(1000);
	for (i = 0; i < 504; i++)
		write_data(pickYourPoison1[i]);
		_delay_ms(1000);
	for (i = 0; i < 504; i++)
		write_data(pickYourPoison2[i]);
		_delay_ms(1000);
	for (i = 0; i < 504; i++)
		write_data(pickYourPoison3[i]);
		_delay_ms(1000);
}
void nokia_lcd_Default(void)
{
	register unsigned i;
	/* Set column and row to 0 */
	write_cmd(0x80);
	write_cmd(0x40);

	/* Write screen to display */
	for (i = 0; i < 504; i++)
		write_data(rough_batman_sketch[i]);
		_delay_ms(2000);
}

void displayAdvertisement(void)
{
	register unsigned i;
	
	write_cmd(0x80);
	write_cmd(0x40);
    
	// display Ads
	for (i = 0; i < 504; i++)
		write_data(TacoBelleducation[i]);
		_delay_ms(2000);
	for (i = 0; i < 504; i++)
		write_data(nikeLike[i]);
		_delay_ms(2000);
	for (i = 0; i < 504; i++)
		write_data(pepsico201510klogo[i]);
		_delay_ms(2000);
	for (i = 0; i < 504; i++)
		write_data(porschePrintAd[i]);
		_delay_ms(2000);
}

void displayLogo(void)
{
	register unsigned i;
	
	write_cmd(0x80);
	write_cmd(0x40);
	
	// display Ads
	for (i = 0; i < 504; i++)
		write_data(Bender[i]);
		_delay_ms(2000);
	for (i = 0; i < 504; i++)
		write_data(LinuxPenguin[i]);
		_delay_ms(2000);
	for (i = 0; i < 504; i++)
		write_data(ucrMascot[i]);
		_delay_ms(2000);
	for (i = 0; i < 504; i++)
		write_data(OverwatchJunkratBombFaceSpray[i]);
		_delay_ms(2000);
}
void displayGifs(void)
{
	register unsigned i;
	
	write_cmd(0x80);
	write_cmd(0x40);
	
	// display Ads
	for (i = 0; i < 504; i++)
		write_data(frame00delay[i]);
		_delay_ms(frameRate);
	for (i = 0; i < 504; i++)
		write_data(frame01delay[i]);
		_delay_ms(frameRate);
	for (i = 0; i < 504; i++)
		write_data(frame02delay[i]);
		_delay_ms(frameRate);
	for (i = 0; i < 504; i++)
		write_data(frame03delay[i]);
		_delay_ms(frameRate);
	for (i = 0; i < 504; i++)
		write_data(frame04delay[i]);
		_delay_ms(frameRate);
	for (i = 0; i < 504; i++)
		write_data(frame05delay[i]);
		_delay_ms(frameRate);
	for (i = 0; i < 504; i++)
		write_data(frame06delay[i]);
		_delay_ms(frameRate);
	for (i = 0; i < 504; i++)
		write_data(frame07delay[i]);
		_delay_ms(frameRate);
	for (i = 0; i < 504; i++)
		write_data(frame08delay[i]);
		_delay_ms(frameRate);
	for (i = 0; i < 504; i++)
		write_data(frame08delay[i]);
		_delay_ms(frameRate);
	for (i = 0; i < 504; i++)
		write_data(frame09delay[i]);
		_delay_ms(frameRate);

}
///////////////////////////////////////////////////////////

