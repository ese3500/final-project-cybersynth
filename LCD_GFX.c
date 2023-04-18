/*
 * LCD_GFX.c
 *
 * Created: 9/20/2021 6:54:25 PM
 *  Author: Alex Dreier
 */ 

#include "LCD_GFX.h"
#include "ST7735.h"

/******************************************************************************
* Local Functions
******************************************************************************/



/******************************************************************************
* Global Functions
******************************************************************************/

/**************************************************************************//**
* @fn			uint16_t rgb565(uint8_t red, uint8_t green, uint8_t blue)
* @brief		Convert RGB888 value to RGB565 16-bit color data
* @note
*****************************************************************************/
uint16_t rgb565(uint8_t red, uint8_t green, uint8_t blue)
{
	return ((((31*(red+4))/255)<<11) | (((63*(green+2))/255)<<5) | ((31*(blue+4))/255));
}

/**************************************************************************//**
* @fn			void LCD_drawPixel(uint8_t x, uint8_t y, uint16_t color)
* @brief		Draw a single pixel of 16-bit rgb565 color to the x & y coordinate
* @note
*****************************************************************************/
void LCD_drawPixel(uint8_t x, uint8_t y, uint16_t color) {
	LCD_setAddr(x,y,x,y);
	SPI_ControllerTx_16bit(color);
}

/**************************************************************************//**
* @fn			void LCD_drawChar(uint8_t x, uint8_t y, uint16_t character, uint16_t fColor, uint16_t bColor)
* @brief		Draw a character starting at the point with foreground and background colors
* @note
*****************************************************************************/
void LCD_drawChar(uint8_t x, uint8_t y, uint16_t character, uint16_t fColor, uint16_t bColor){
	uint16_t row = character - 0x20;		//Determine row of ASCII table starting at space
	int i, j;
	if ((LCD_WIDTH-x>7)&&(LCD_HEIGHT-y>7)){
		for(i=0;i<5;i++){
			uint8_t pixels = ASCII[row][i]; //Go through the list of pixels
			for(j=0;j<8;j++){
				if ((pixels>>j)&1==1){
					LCD_drawPixel(x+i,y+j,fColor);
				}
				else {
					LCD_drawPixel(x+i,y+j,bColor);
				}
			}
		}
	}
}


/******************************************************************************
* LAB 4 TO DO. COMPLETE THE FUNCTIONS BELOW.
* You are free to create and add any additional files, libraries, and/or
*  helper function. All code must be authentically yours.
******************************************************************************/

/**************************************************************************//**
* @fn			void LCD_drawCircle(uint8_t x0, uint8_t y0, uint8_t radius,uint16_t color)
* @brief		Draw a colored circle of set radius at coordinates
* @note
*****************************************************************************/
void LCD_drawCircle(uint8_t x0, uint8_t y0, uint8_t radius,uint16_t color)
{
	int const r2 = radius * radius;
	int const left = (x0 - radius) < 0 ? -x0 : -radius;
	int const right = (x0 + radius) > (LCD_WIDTH - 1) ? LCD_WIDTH - x0 - 1 : radius;
	int const top = (y0 - radius) < 0 ? -y0 : -radius;
	int const bottom = (y0 + radius) > (LCD_HEIGHT - 1) ? LCD_HEIGHT - y0 - 1 : radius;
	
	int firstX = 0;
	int lastX = 0;
	int within = 0;
	int start = 0;
	int y2 = 0;
	int x2 = 0;
	for (int y = top; y <= bottom; y++) {
		firstX = 0;
		lastX = 0;
		start = 0;
		if (y >= 0) {
			y2 = (y - .5) * (y - .5);
		} else {
			y2 = (y + .5) * (y + .5);
		}
		for (int x = left; x <= right; x++) {
			if (x >= 0) {
				x2 = (x - .5) * (x - .5);
			} else {
				x2 = (x + .5) * (x + .5);
			}
			
			within = x2 + y2 <= r2;
			
			if (within && !start) {
				start = 1;
				firstX = x;
			}
			
			if (within && start) {
				lastX = x;
			}
			
			if (!within && start) {
				break;
			}
		}
		
		LCD_drawBlock(firstX + x0, y + y0, lastX + x0, y + y0, color);
	} 
}


/**************************************************************************//**
* @fn			void LCD_drawLine(short x0,short y0,short x1,short y1,uint16_t c)
* @brief		Draw a line from and to a point with a color
* @note
*****************************************************************************/
void LCD_drawLine(short x0,short y0,short x1,short y1,uint16_t c)
{
	if (x0 > x1) {
		LCD_drawLine(x1, y1, x0, y0, c);
	} else {
		if ((x0 == x1) || (y0 == y1)) {
			if (y0 <= y1) {
				LCD_setAddr(x0, y0, x1, y1);
			} else {
				LCD_setAddr(x1, y1, x0, y0);
			}
			int num = (x1 - x0 + 1) * (y1 - y0 + 1);
			num *= ((num > 0) - (num < 0)); // gets absolute val
			for (int i = 0; i < num; i++) {
				SPI_ControllerTx_16bit(c);
			}
		} else {
			LCD_drawPixel(x0, y0, c);
			double const m = ((double) (y1 - y0)) / ((x1 - x0));
			int x = x0;
			int y = y0;
			int lastX = x0;
			int lastY = y0;
			double yPrev = y0;
			double dY;
			double absdY;
			double offset;
			if (m <= 1 && m >= -1) { 
				// draw pixels until it reaches the endpoint
				while (x < x1) {
					// find first x at different y value
					x++;
					dY = m * (x - lastX);
					absdY = dY * ((dY > 0) - (dY < 0)); // abs value
					offset = m > 0 ? y - yPrev : yPrev - y;
					if ((absdY - offset) >= .5) {
						y = m > 0 ? y + 1 : y - 1;
						yPrev += dY;
						LCD_drawPixel(x, y, c);
						LCD_drawBlock(lastX + 1, lastY, x - 1, lastY, c);
						lastX = x;
						lastY = y;
					}
				}
			} else {
				// draw pixels until it reaches the endpoint
				while (x < x1) {
					// find first x at different y value
					x++;
					dY = m * (x - lastX);
					absdY = dY * ((dY > 0) - (dY < 0)); // abs value
					offset = m > 0 ? y - yPrev : yPrev - y;
					if ((absdY - offset) >= .5) {
						double roundY = y + dY + (m < 0 ? offset : 0 - offset);
						y += dY + (m < 0 ? offset : 0 - offset);
						yPrev += dY;
						
						// fix rounding error
						if (roundY - y >= .5) {
							y++;
						}
						
						LCD_drawPixel(x, y, c);
						if (m > 0) {
							LCD_drawBlock(lastX, lastY, lastX, y - 1, c);
						} else {
							LCD_drawBlock(lastX, y + 1, lastX, lastY, c);
						}
						
						lastX = x;
						lastY = y;
					}
				}
			}
			
			
			if (m > 0) {
				LCD_drawBlock(lastX, lastY, x1, y1, c);
				} else {
				LCD_drawBlock(lastX, y1, x1, lastY, c);
			}
			LCD_drawPixel(x1, y1, c);
		}
	}
}



/**************************************************************************//**
* @fn			void LCD_drawBlock(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1,uint16_t color)
* @brief		Draw a colored block at coordinates
* @note
*****************************************************************************/
void LCD_drawBlock(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1,uint16_t color)
{
	LCD_setAddr(x0,y0,x1,y1);
	int const num = (x1 - x0 + 1) * (y1 - y0 + 1);
	for (int i = 0; i < num; i++) {
		SPI_ControllerTx_16bit(color);
	}
}

/**************************************************************************//**
* @fn			void LCD_setScreen(uint16_t color)
* @brief		Draw the entire screen to a color
* @note
*****************************************************************************/
void LCD_setScreen(uint16_t color) 
{
	LCD_drawBlock(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1, color);
}

/**************************************************************************//**
* @fn			void LCD_drawString(uint8_t x, uint8_t y, char* str, uint16_t fg, uint16_t bg)
* @brief		Draw a string starting at the point with foreground and background colors
* @note
*****************************************************************************/
void LCD_drawString(uint8_t x, uint8_t y, char* str, uint16_t fg, uint16_t bg)
{
	while(*str != '\0') {
		LCD_drawChar(x, y, *str, fg, bg);
		x += 6;
		str++;
	}
}