#ifndef __LCD128_H__
#define __LCD128_H__

#include <stdint.h>
#include <stdbool.h>
#include "fonts.h"
#include "stm32f1xx_hal.h"

#define LCD128_WIDTH  240
#define LCD128_HEIGHT 240

// ==== Struct chứa thông tin LCD ==== 
typedef struct {
    SPI_HandleTypeDef* hspi;
    GPIO_TypeDef* CS_Port;
    uint16_t      CS_Pin;
    GPIO_TypeDef* DC_Port;
    uint16_t      DC_Pin;
    GPIO_TypeDef* RST_Port;
    uint16_t      RST_Pin;
} LCD128_HandleTypeDef;

// ==== Màu sắc cơ bản ====
#define LCD128_BLACK   0x0000
#define LCD128_BLUE    0x001F
#define LCD128_RED     0xF800
#define LCD128_GREEN   0x07E0
#define LCD128_CYAN    0x07FF
#define LCD128_MAGENTA 0xF81F
#define LCD128_YELLOW  0xFFE0
#define LCD128_WHITE   0xFFFF

#define LCD128_COLOR565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))

#ifdef __cplusplus
extern "C" {
#endif

void LCD128_Init(LCD128_HandleTypeDef* lcd);
void LCD128_DrawPixel(LCD128_HandleTypeDef* lcd, uint16_t x, uint16_t y, uint16_t color);
void LCD128_WriteString(LCD128_HandleTypeDef* lcd, uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor);
void LCD128_FillRectangle(LCD128_HandleTypeDef* lcd, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void LCD128_FillScreen(LCD128_HandleTypeDef* lcd, uint16_t color);
void LCD128_DrawImage(LCD128_HandleTypeDef* lcd, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data);
void LCD128_DrawImage_DMA(LCD128_HandleTypeDef* lcd, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data);
void LCD128_InvertColors(LCD128_HandleTypeDef* lcd, bool invert);
void LCD128_SetBackLight(LCD128_HandleTypeDef* lcd, uint16_t value);
void LCD128_SetAddressWindow(LCD128_HandleTypeDef* lcd, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
#ifdef __cplusplus
}
#endif

#endif // __LCD128_H__ 
