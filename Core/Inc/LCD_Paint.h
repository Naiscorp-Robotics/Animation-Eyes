#ifndef __LCD_PAINT_H__
#define __LCD_PAINT_H__

#include "lcd128.h"
#include <stdint.h>

void LCD_Paint_DrawPixel(LCD128_HandleTypeDef* lcd, int16_t x, int16_t y, uint16_t color);
void LCD_Paint_WriteLine(LCD128_HandleTypeDef* lcd, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void LCD_Paint_DrawFastVLine(LCD128_HandleTypeDef* lcd, int16_t x, int16_t y, int16_t h, uint16_t color);
void LCD_Paint_DrawFastHLine(LCD128_HandleTypeDef* lcd, int16_t x, int16_t y, int16_t w, uint16_t color);
void LCD_Paint_FillRect(LCD128_HandleTypeDef* lcd, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void LCD_Paint_DrawLine(LCD128_HandleTypeDef* lcd, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void LCD_Paint_DrawCircle(LCD128_HandleTypeDef* lcd, int16_t x0, int16_t y0, int16_t r, uint16_t color);
void LCD_Paint_DrawCircleHelper(LCD128_HandleTypeDef* lcd, int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color);
void LCD_Paint_FillCircleHelper(LCD128_HandleTypeDef* lcd, int16_t x0, int16_t y0, int16_t r, uint8_t corners, int16_t delta, uint16_t color);
void LCD_Paint_FillCircle(LCD128_HandleTypeDef* lcd, int16_t x0, int16_t y0, int16_t r, uint16_t color);
void LCD_Paint_DrawRect(LCD128_HandleTypeDef* lcd, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void LCD_Paint_DrawRoundRect(LCD128_HandleTypeDef* lcd, int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
void LCD_Paint_FillRoundRect(LCD128_HandleTypeDef* lcd, int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
void LCD_Paint_DrawTriangle(LCD128_HandleTypeDef* lcd, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
void LCD_Paint_FillTriangle(LCD128_HandleTypeDef* lcd, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);

#endif 