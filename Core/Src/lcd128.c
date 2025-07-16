#include "lcd128.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// ==== Lệnh GC9A01 ====
#define LCD128_SWRESET 0x01
#define LCD128_SLPOUT  0x11
#define LCD128_DISPON  0x29
#define LCD128_CASET   0x2A
#define LCD128_RASET   0x2B
#define LCD128_RAMWR   0x2C
#define LCD128_MADCTL  0x36
#define LCD128_COLMOD  0x3A
#define LCD128_INVON   0x21
#define LCD128_INVOFF  0x20

static void LCD128_CS_0(LCD128_HandleTypeDef* lcd) {
    HAL_GPIO_WritePin(lcd->CS_Port, lcd->CS_Pin, GPIO_PIN_RESET);
}
static void LCD128_CS_1(LCD128_HandleTypeDef* lcd) {
    HAL_GPIO_WritePin(lcd->CS_Port, lcd->CS_Pin, GPIO_PIN_SET);
}
static void LCD128_DC_0(LCD128_HandleTypeDef* lcd) {
    HAL_GPIO_WritePin(lcd->DC_Port, lcd->DC_Pin, GPIO_PIN_RESET);
}
static void LCD128_DC_1(LCD128_HandleTypeDef* lcd) {
    HAL_GPIO_WritePin(lcd->DC_Port, lcd->DC_Pin, GPIO_PIN_SET);
}
static void LCD128_RST_0(LCD128_HandleTypeDef* lcd) {
    HAL_GPIO_WritePin(lcd->RST_Port, lcd->RST_Pin, GPIO_PIN_RESET);
}
static void LCD128_RST_1(LCD128_HandleTypeDef* lcd) {
    HAL_GPIO_WritePin(lcd->RST_Port, lcd->RST_Pin, GPIO_PIN_SET);
}
static void LCD128_SPI_WRITE(LCD128_HandleTypeDef* lcd, uint8_t byte) {
    HAL_SPI_Transmit(lcd->hspi, &byte, 1, HAL_MAX_DELAY);
}
static void LCD128_DELAY(uint32_t ms) {
    HAL_Delay(ms);
}

static void LCD128_WriteCommand(LCD128_HandleTypeDef* lcd, uint8_t cmd) {
    LCD128_DC_0(lcd);
    LCD128_CS_0(lcd);
    LCD128_SPI_WRITE(lcd, cmd);
    LCD128_CS_1(lcd);
}

static void LCD128_WriteData8(LCD128_HandleTypeDef* lcd, uint8_t data) {
    LCD128_DC_1(lcd);
    LCD128_CS_0(lcd);
    LCD128_SPI_WRITE(lcd, data);
    LCD128_CS_1(lcd);
}

static void LCD128_WriteData16(LCD128_HandleTypeDef* lcd, uint16_t data) {
    LCD128_DC_1(lcd);
    LCD128_CS_0(lcd);
    uint8_t hi = data >> 8, lo = data & 0xFF;
    LCD128_SPI_WRITE(lcd, hi);
    LCD128_SPI_WRITE(lcd, lo);
    LCD128_CS_1(lcd);
}

static void LCD128_WriteDataN(LCD128_HandleTypeDef* lcd, const uint8_t* buff, size_t size) {
    LCD128_DC_1(lcd);
    LCD128_CS_0(lcd);
    for (size_t i = 0; i < size; i++) {
        LCD128_SPI_WRITE(lcd, buff[i]);
    }
    LCD128_CS_1(lcd);
}

static void LCD128_Reset(LCD128_HandleTypeDef* lcd) {
    LCD128_RST_1(lcd);
    LCD128_DELAY(100);
    LCD128_RST_0(lcd);
    LCD128_DELAY(100);
    LCD128_RST_1(lcd);
    LCD128_DELAY(100);
}

static void LCD128_SetAddressWindow(LCD128_HandleTypeDef* lcd, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    LCD128_WriteCommand(lcd, LCD128_CASET);
    LCD128_WriteData8(lcd, x0 >> 8);
    LCD128_WriteData8(lcd, x0 & 0xFF);
    LCD128_WriteData8(lcd, x1 >> 8);
    LCD128_WriteData8(lcd, x1 & 0xFF);

    LCD128_WriteCommand(lcd, LCD128_RASET);
    LCD128_WriteData8(lcd, y0 >> 8);
    LCD128_WriteData8(lcd, y0 & 0xFF);
    LCD128_WriteData8(lcd, y1 >> 8);
    LCD128_WriteData8(lcd, y1 & 0xFF);

    LCD128_WriteCommand(lcd, LCD128_RAMWR);
}

void LCD128_Init(LCD128_HandleTypeDef* lcd) {
    LCD128_Reset(lcd);
    // Chuỗi lệnh khởi tạo GC9A01 (tham khảo LCD_1in28.c)
    LCD128_WriteCommand(lcd, 0xEF);
    LCD128_WriteCommand(lcd, 0xEB); LCD128_WriteData8(lcd, 0x14);
    LCD128_WriteCommand(lcd, 0xFE);
    LCD128_WriteCommand(lcd, 0xEF);
    LCD128_WriteCommand(lcd, 0xEB); LCD128_WriteData8(lcd, 0x14);
    LCD128_WriteCommand(lcd, 0x84); LCD128_WriteData8(lcd, 0x40);
    LCD128_WriteCommand(lcd, 0x85); LCD128_WriteData8(lcd, 0xFF);
    LCD128_WriteCommand(lcd, 0x86); LCD128_WriteData8(lcd, 0xFF);
    LCD128_WriteCommand(lcd, 0x87); LCD128_WriteData8(lcd, 0xFF);
    LCD128_WriteCommand(lcd, 0x88); LCD128_WriteData8(lcd, 0x0A);
    LCD128_WriteCommand(lcd, 0x89); LCD128_WriteData8(lcd, 0x21);
    LCD128_WriteCommand(lcd, 0x8A); LCD128_WriteData8(lcd, 0x00);
    LCD128_WriteCommand(lcd, 0x8B); LCD128_WriteData8(lcd, 0x80);
    LCD128_WriteCommand(lcd, 0x8C); LCD128_WriteData8(lcd, 0x01);
    LCD128_WriteCommand(lcd, 0x8D); LCD128_WriteData8(lcd, 0x01);
    LCD128_WriteCommand(lcd, 0x8E); LCD128_WriteData8(lcd, 0xFF);
    LCD128_WriteCommand(lcd, 0x8F); LCD128_WriteData8(lcd, 0xFF);
    LCD128_WriteCommand(lcd, 0xB6); LCD128_WriteData8(lcd, 0x00); LCD128_WriteData8(lcd, 0x20);
    LCD128_WriteCommand(lcd, 0x36); LCD128_WriteData8(lcd, 0x08); // vertical
    LCD128_WriteCommand(lcd, 0x3A); LCD128_WriteData8(lcd, 0x05);
    LCD128_WriteCommand(lcd, 0x90); LCD128_WriteData8(lcd, 0x08); LCD128_WriteData8(lcd, 0x08); LCD128_WriteData8(lcd, 0x08); LCD128_WriteData8(lcd, 0x08); LCD128_WriteData8(lcd, 0x08);
    LCD128_WriteCommand(lcd, 0xBD); LCD128_WriteData8(lcd, 0x06);
    LCD128_WriteCommand(lcd, 0xBC); LCD128_WriteData8(lcd, 0x00);
    LCD128_WriteCommand(lcd, 0xFF); LCD128_WriteData8(lcd, 0x60); LCD128_WriteData8(lcd, 0x01); LCD128_WriteData8(lcd, 0x04);
    LCD128_WriteCommand(lcd, 0xC3); LCD128_WriteData8(lcd, 0x13);
    LCD128_WriteCommand(lcd, 0xC4); LCD128_WriteData8(lcd, 0x13);
    LCD128_WriteCommand(lcd, 0xC9); LCD128_WriteData8(lcd, 0x22);
    LCD128_WriteCommand(lcd, 0xBE); LCD128_WriteData8(lcd, 0x11);
    LCD128_WriteCommand(lcd, 0xE1); LCD128_WriteData8(lcd, 0x10); LCD128_WriteData8(lcd, 0x0E);
    LCD128_WriteCommand(lcd, 0xDF); LCD128_WriteData8(lcd, 0x21); LCD128_WriteData8(lcd, 0x0c); LCD128_WriteData8(lcd, 0x02);
    LCD128_WriteCommand(lcd, 0xF0); LCD128_WriteData8(lcd, 0x45); LCD128_WriteData8(lcd, 0x09); LCD128_WriteData8(lcd, 0x08); LCD128_WriteData8(lcd, 0x08); LCD128_WriteData8(lcd, 0x26); LCD128_WriteData8(lcd, 0x2A);
    LCD128_WriteCommand(lcd, 0xF1); LCD128_WriteData8(lcd, 0x43); LCD128_WriteData8(lcd, 0x70); LCD128_WriteData8(lcd, 0x72); LCD128_WriteData8(lcd, 0x36); LCD128_WriteData8(lcd, 0x37); LCD128_WriteData8(lcd, 0x6F);
    LCD128_WriteCommand(lcd, 0xF2); LCD128_WriteData8(lcd, 0x45); LCD128_WriteData8(lcd, 0x09); LCD128_WriteData8(lcd, 0x08); LCD128_WriteData8(lcd, 0x08); LCD128_WriteData8(lcd, 0x26); LCD128_WriteData8(lcd, 0x2A);
    LCD128_WriteCommand(lcd, 0xF3); LCD128_WriteData8(lcd, 0x43); LCD128_WriteData8(lcd, 0x70); LCD128_WriteData8(lcd, 0x72); LCD128_WriteData8(lcd, 0x36); LCD128_WriteData8(lcd, 0x37); LCD128_WriteData8(lcd, 0x6F);
    LCD128_WriteCommand(lcd, 0xED); LCD128_WriteData8(lcd, 0x1B); LCD128_WriteData8(lcd, 0x0B);
    LCD128_WriteCommand(lcd, 0xAE); LCD128_WriteData8(lcd, 0x77);
    LCD128_WriteCommand(lcd, 0xCD); LCD128_WriteData8(lcd, 0x63);
    LCD128_WriteCommand(lcd, 0x70); LCD128_WriteData8(lcd, 0x07); LCD128_WriteData8(lcd, 0x07); LCD128_WriteData8(lcd, 0x04); LCD128_WriteData8(lcd, 0x0E); LCD128_WriteData8(lcd, 0x0F); LCD128_WriteData8(lcd, 0x09); LCD128_WriteData8(lcd, 0x07); LCD128_WriteData8(lcd, 0x08); LCD128_WriteData8(lcd, 0x03);
    LCD128_WriteCommand(lcd, 0xE8); LCD128_WriteData8(lcd, 0x34);
    LCD128_WriteCommand(lcd, 0x62); LCD128_WriteData8(lcd, 0x18); LCD128_WriteData8(lcd, 0x0D); LCD128_WriteData8(lcd, 0x71); LCD128_WriteData8(lcd, 0xED); LCD128_WriteData8(lcd, 0x70); LCD128_WriteData8(lcd, 0x70); LCD128_WriteData8(lcd, 0x18); LCD128_WriteData8(lcd, 0x0F); LCD128_WriteData8(lcd, 0x71); LCD128_WriteData8(lcd, 0xEF); LCD128_WriteData8(lcd, 0x70); LCD128_WriteData8(lcd, 0x70);
    LCD128_WriteCommand(lcd, 0x63); LCD128_WriteData8(lcd, 0x18); LCD128_WriteData8(lcd, 0x11); LCD128_WriteData8(lcd, 0x71); LCD128_WriteData8(lcd, 0xF1); LCD128_WriteData8(lcd, 0x70); LCD128_WriteData8(lcd, 0x70); LCD128_WriteData8(lcd, 0x18); LCD128_WriteData8(lcd, 0x13); LCD128_WriteData8(lcd, 0x71); LCD128_WriteData8(lcd, 0xF3); LCD128_WriteData8(lcd, 0x70); LCD128_WriteData8(lcd, 0x70);
    LCD128_WriteCommand(lcd, 0x64); LCD128_WriteData8(lcd, 0x28); LCD128_WriteData8(lcd, 0x29); LCD128_WriteData8(lcd, 0xF1); LCD128_WriteData8(lcd, 0x01); LCD128_WriteData8(lcd, 0xF1); LCD128_WriteData8(lcd, 0x00); LCD128_WriteData8(lcd, 0x07);
    LCD128_WriteCommand(lcd, 0x66); LCD128_WriteData8(lcd, 0x3C); LCD128_WriteData8(lcd, 0x00); LCD128_WriteData8(lcd, 0xCD); LCD128_WriteData8(lcd, 0x67); LCD128_WriteData8(lcd, 0x45); LCD128_WriteData8(lcd, 0x45); LCD128_WriteData8(lcd, 0x10); LCD128_WriteData8(lcd, 0x00); LCD128_WriteData8(lcd, 0x00); LCD128_WriteData8(lcd, 0x00);
    LCD128_WriteCommand(lcd, 0x67); LCD128_WriteData8(lcd, 0x00); LCD128_WriteData8(lcd, 0x3C); LCD128_WriteData8(lcd, 0x00); LCD128_WriteData8(lcd, 0x00); LCD128_WriteData8(lcd, 0x00); LCD128_WriteData8(lcd, 0x01); LCD128_WriteData8(lcd, 0x54); LCD128_WriteData8(lcd, 0x10); LCD128_WriteData8(lcd, 0x32); LCD128_WriteData8(lcd, 0x98);
    LCD128_WriteCommand(lcd, 0x74); LCD128_WriteData8(lcd, 0x10); LCD128_WriteData8(lcd, 0x85); LCD128_WriteData8(lcd, 0x80); LCD128_WriteData8(lcd, 0x00); LCD128_WriteData8(lcd, 0x00); LCD128_WriteData8(lcd, 0x4E); LCD128_WriteData8(lcd, 0x00);
    LCD128_WriteCommand(lcd, 0x98); LCD128_WriteData8(lcd, 0x3e); LCD128_WriteData8(lcd, 0x07);
    LCD128_WriteCommand(lcd, 0x35);
    LCD128_WriteCommand(lcd, 0x21);
    LCD128_WriteCommand(lcd, LCD128_SLPOUT);
    LCD128_DELAY(120);
    LCD128_WriteCommand(lcd, LCD128_DISPON);
    LCD128_DELAY(20);
}

void LCD128_DrawPixel(LCD128_HandleTypeDef* lcd, uint16_t x, uint16_t y, uint16_t color) {
    if (x >= LCD128_WIDTH || y >= LCD128_HEIGHT) return;
    LCD128_SetAddressWindow(lcd, x, y, x, y);
    LCD128_WriteData16(lcd, color);
}

static void LCD128_WriteChar(LCD128_HandleTypeDef* lcd, uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor) {
    uint32_t i, b, j;
    LCD128_SetAddressWindow(lcd, x, y, x + font.width - 1, y + font.height - 1);
    for (i = 0; i < font.height; i++) {
        b = font.data[(ch - 32) * font.height + i];
        for (j = 0; j < font.width; j++) {
            uint16_t pixel = ((b << j) & 0x8000) ? color : bgcolor;
            LCD128_WriteData16(lcd, pixel);
        }
    }
}

void LCD128_WriteString(LCD128_HandleTypeDef* lcd, uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor) {
    while (*str) {
        if (x + font.width > LCD128_WIDTH) {
            x = 0;
            y += font.height;
            if (y + font.height > LCD128_HEIGHT) break;
        }
        LCD128_WriteChar(lcd, x, y, *str, font, color, bgcolor);
        x += font.width;
        str++;
    }
}

void LCD128_FillRectangle(LCD128_HandleTypeDef* lcd, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if ((x >= LCD128_WIDTH) || (y >= LCD128_HEIGHT)) return;
    if ((x + w - 1) >= LCD128_WIDTH) w = LCD128_WIDTH - x;
    if ((y + h - 1) >= LCD128_HEIGHT) h = LCD128_HEIGHT - y;
    LCD128_SetAddressWindow(lcd, x, y, x + w - 1, y + h - 1);
    for (uint32_t i = 0; i < w * h; i++) {
        LCD128_WriteData16(lcd, color);
    }
}

void LCD128_FillScreen(LCD128_HandleTypeDef* lcd, uint16_t color) {
    LCD128_FillRectangle(lcd, 0, 0, LCD128_WIDTH, LCD128_HEIGHT, color);
}

void LCD128_DrawImage(LCD128_HandleTypeDef* lcd, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data) {
    if ((x >= LCD128_WIDTH) || (y >= LCD128_HEIGHT)) return;
    if ((x + w - 1) >= LCD128_WIDTH) return;
    if ((y + h - 1) >= LCD128_HEIGHT) return;
    LCD128_SetAddressWindow(lcd, x, y, x + w - 1, y + h - 1);
    LCD128_DC_1(lcd);
    LCD128_CS_0(lcd);
    for (uint32_t i = 0; i < w * h; i++) {
        uint8_t hi = data[i] >> 8, lo = data[i] & 0xFF;
        LCD128_SPI_WRITE(lcd, hi);
        LCD128_SPI_WRITE(lcd, lo);
    }
    LCD128_CS_1(lcd);
}

void LCD128_InvertColors(LCD128_HandleTypeDef* lcd, bool invert) {
    LCD128_WriteCommand(lcd, invert ? LCD128_INVON : LCD128_INVOFF);
}

void LCD128_SetBackLight(LCD128_HandleTypeDef* lcd, uint16_t value) {
    // Tùy vào phần cứng, bạn có thể điều khiển backlight ở đây
    // Ví dụ: HAL_TIM_PWM_SetCompare(...)
    (void)lcd; // tránh warning nếu chưa dùng
    (void)value;
} 