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

// // ==== Màu sắc cơ bản ====
// #define LCD128_BLACK   0x0000
// #define LCD128_BLUE    0x001F
// #define LCD128_RED     0xF800
// #define LCD128_GREEN   0x07E0
// #define LCD128_CYAN    0x07FF
// #define LCD128_MAGENTA 0xF81F
// #define LCD128_YELLOW  0xFFE0
// #define LCD128_WHITE   0xFFFF
// // ==== Màu cơ bản đã có ====
// // (giữ nguyên phần bạn đã viết)
// // ==== Màu sắc cơ bản ====
// #define GC9A01A_BLACK       0x0000
// #define GC9A01A_NAVY        0x000F
// #define GC9A01A_DARKGREEN   0x03E0
// #define GC9A01A_DARKCYAN    0x03EF
// #define GC9A01A_MAROON      0x7800
// #define GC9A01A_PURPLE      0x780F
// #define GC9A01A_OLIVE       0x7BE0
// #define GC9A01A_LIGHTGREY   0xC618
// #define GC9A01A_DARKGREY    0x7BEF
// #define GC9A01A_BLUE        0x001F
// #define GC9A01A_GREEN       0x07E0
// #define GC9A01A_CYAN        0x07FF
// #define GC9A01A_RED         0xF800
// #define GC9A01A_MAGENTA     0xF81F
// #define GC9A01A_YELLOW      0xFFE0
// #define GC9A01A_WHITE       0xFFFF
// #define GC9A01A_ORANGE      0xFD20
// #define GC9A01A_GREENYELLOW 0xAFE5
// #define GC9A01A_PINK        0xFC18
// #define GC9A01A_SKYBLUE     0x867D  // Xanh da trời nhạt
// #define GC9A01A_LIGHTBLUE   0xAEDC  // Xanh dương nhạt
// #define GC9A01A_BABYBLUE    0xB6DF  // Xanh baby
// #define GC9A01A_LIGHTGREEN  0x97F6  // Xanh lá nhạt
// #define GC9A01A_MINT        0xB7F6  // Xanh bạc hà
// #define GC9A01A_LIME        0x07FF  // Xanh chanh
// #define GC9A01A_IVORY       0xFFFE  // Trắng ngà
// #define GC9A01A_LIGHTPINK   0xFD9F  // Hồng nhạt
// #define GC9A01A_PEACHPUFF   0xFDB8  // Cam đào
// #define GC9A01A_BEIGE       0xF7BB  // Màu be
// #define GC9A01A_LAVENDER    0xD69A  // Tím oải hương
// #define GC9A01A_SALMON      0xFA60  // Màu cá hồi

// dùng thường
// #define LCD128_COLOR565(r, g, b) \
//   (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))
// dùng cho dma
#define LCD128_COLOR565(g, r, b) \
  (((b & 0xF8) << 8) | ((g & 0xFC) << 3) | ((r & 0xF8) >> 3))

// ==== Màu cơ bản ====
#define LCD128_BLACK        LCD128_COLOR565(0, 0, 0)         // Đen
#define LCD128_BLUE         LCD128_COLOR565(0, 0, 255)       // Xanh dương
#define LCD128_GREEN        LCD128_COLOR565(0, 255, 0)       // Xanh lá
#define LCD128_CYAN         LCD128_COLOR565(0, 170, 255)     // Xanh cyan (ngọc lam)
#define LCD128_RED          LCD128_COLOR565(255, 0, 0)       // Đỏ
#define LCD128_MAGENTA      LCD128_COLOR565(255, 0, 255)     // Tím hồng (Magenta)
#define LCD128_YELLOW       LCD128_COLOR565(255, 255, 0)     // Vàng
#define LCD128_WHITE        LCD128_COLOR565(255, 255, 255)   // Trắng

// ==== Màu mở rộng ====
#define LCD128_NAVY         LCD128_COLOR565(0, 0, 128)       // Xanh hải quân
#define LCD128_DARKGREEN    LCD128_COLOR565(0, 128, 0)       // Xanh lá đậm
#define LCD128_DARKCYAN     LCD128_COLOR565(0, 128, 128)     // Xanh cyan đậm
#define LCD128_MAROON       LCD128_COLOR565(128, 0, 0)       // Đỏ đậm (maroon)
#define LCD128_PURPLE       LCD128_COLOR565(128, 0, 128)     // Tím
#define LCD128_OLIVE        LCD128_COLOR565(128, 128, 0)     // Vàng ô-liu
#define LCD128_LIGHTGREY    LCD128_COLOR565(192, 192, 192)   // Xám nhạt
#define LCD128_DARKGREY     LCD128_COLOR565(128, 128, 128)   // Xám đậm

#define LCD128_ORANGE       LCD128_COLOR565(255, 165, 0)     // Cam
#define LCD128_GREENYELLOW  LCD128_COLOR565(173, 255, 47)    // Vàng xanh lá
#define LCD128_PINK         LCD128_COLOR565(255, 192, 203)   // Hồng
#define LCD128_LIGHTPINK    LCD128_COLOR565(255, 182, 193)   // Hồng nhạt
#define LCD128_PEACHPUFF    LCD128_COLOR565(255, 218, 185)   // Cam đào
#define LCD128_BEIGE        LCD128_COLOR565(245, 245, 220)   // Màu be
#define LCD128_LAVENDER     LCD128_COLOR565(230, 230, 250)   // Tím oải hương
#define LCD128_SALMON       LCD128_COLOR565(250, 128, 114)   // Cam cá hồi

#define LCD128_SKYBLUE      LCD128_COLOR565(135, 206, 235)   // Xanh da trời
#define LCD128_LIGHTBLUE    LCD128_COLOR565(173, 216, 230)   // Xanh dương nhạt
#define LCD128_BABYBLUE     LCD128_COLOR565(188, 227, 240)   // Xanh baby
#define LCD128_LIGHTGREEN   LCD128_COLOR565(144, 238, 144)   // Xanh lá nhạt
#define LCD128_MINT         LCD128_COLOR565(181, 255, 218)   // Xanh bạc hà
#define LCD128_LIME         LCD128_COLOR565(0, 255, 127)     // Xanh chanh
#define LCD128_IVORY        LCD128_COLOR565(255, 255, 240)   // Trắng ngà

#define LCD128_GRAY11   LCD128_COLOR565(28, 28, 28)   // Xám đậm


#define EYE_BLUE_1   LCD128_COLOR565(0, 100, 255)   // Xanh dương sáng (vòng tròng mắt) 144 100 50 0
#define EYE_BLUE_2   LCD128_COLOR565(176, 224, 230)   // Xanh dương sáng (sắc độ khác)
#define EYE_BLUE_3   LCD128_COLOR565(205, 133, 63)    // Xanh dương sáng (sắc độ khác)
#define EYE_BLUE_4   LCD128_COLOR565(98, 145, 166)    // Xanh dương sáng (vùng sáng nhất)
#define EYE_BLUE_5   LCD128_COLOR565(90, 137, 158)    // Xanh dương sáng (sắc độ khác)


//  #define LCD128_RED   LCD128_COLOR565(255, 0, 0)
// #define LCD128_GREEN LCD128_COLOR565(0, 255, 0)
// #define LCD128_BLUE  LCD128_COLOR565(0, 0, 255)
// #define LCD128_COLOR565(r, g, b) (((b & 0xF8) >> 3) | ((r & 0xF8) << 8) | ((g & 0xFC) << 3))
// #define LCD128_COLOR565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3)) // Chuẩn RGB565
// #define LCD128_COLOR565(r, g, b) (((b & 0xF8) << 8) | ((g & 0xFC) << 3) | ((r & 0xF8) >> 3)) // Đảo R ↔ B nếu màn hình dùng BGR
  // Đảo R và B nếu LCD nhận BGR565
// #define LCD128_COLOR565(b, r, g) (((b & 0xF8) << 8) | ((g & 0xFC) << 3) | ((r & 0xF8) >> 3))
// #define LCD128_BLUE   LCD128_COLOR565(255, 0, 0)
// #define LCD128_GREEN LCD128_COLOR565(0, 255, 0)
// #define LCD128_RED  LCD128_COLOR565(0, 0, 255)
// #define LCD128_RED   0x001F  // Đỏ thành xanh dương
// #define LCD128_GREEN 0x07E0  // Giữ nguyên
// #define LCD128_BLUE  0xF800  // Xanh dương thành đỏ

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
