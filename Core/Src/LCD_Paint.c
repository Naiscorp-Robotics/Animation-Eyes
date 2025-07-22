#include "LCD_Paint.h"
#include <stdlib.h>
#include <math.h>
#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }
#define min(a, b) (((a) < (b)) ? (a) : (b))

void LCD_Paint_DrawPixel(LCD128_HandleTypeDef* lcd, int16_t x, int16_t y, uint16_t color) {
    LCD128_DrawPixel(lcd, x, y, color);
}

void LCD_Paint_FillRect(LCD128_HandleTypeDef* lcd, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    LCD128_FillRectangle(lcd, x, y, w, h, color);
}

void LCD_Paint_WritePixel(LCD128_HandleTypeDef* lcd, int16_t x, int16_t y, uint16_t color) {
    LCD_Paint_DrawPixel(lcd, x, y, color);
}

void LCD_Paint_WriteLine(LCD128_HandleTypeDef* lcd, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        _swap_int16_t(x0, y0);
        _swap_int16_t(x1, y1);
    }
    if (x0 > x1) {
        _swap_int16_t(x0, x1);
        _swap_int16_t(y0, y1);
    }
    int16_t dx = x1 - x0;
    int16_t dy = abs(y1 - y0);
    int16_t err = dx / 2;
    int16_t ystep = (y0 < y1) ? 1 : -1;
    for (; x0 <= x1; x0++) {
        if (steep) {
            LCD_Paint_WritePixel(lcd, y0, x0, color);
        } else {
            LCD_Paint_WritePixel(lcd, x0, y0, color);
        }
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
}

void LCD_Paint_DrawFastVLine(LCD128_HandleTypeDef* lcd, int16_t x, int16_t y, int16_t h, uint16_t color) {
    LCD_Paint_WriteLine(lcd, x, y, x, y + h - 1, color);
}
void LCD_Paint_DrawFastHLine(LCD128_HandleTypeDef* lcd, int16_t x, int16_t y, int16_t w, uint16_t color) {
    LCD_Paint_WriteLine(lcd, x, y, x + w - 1, y, color);
}

void LCD_Paint_DrawLine(LCD128_HandleTypeDef* lcd, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    if (x0 == x1) {
        if (y0 > y1) _swap_int16_t(y0, y1);
        LCD_Paint_DrawFastVLine(lcd, x0, y0, y1 - y0 + 1, color);
    } else if (y0 == y1) {
        if (x0 > x1) _swap_int16_t(x0, x1);
        LCD_Paint_DrawFastHLine(lcd, x0, y0, x1 - x0 + 1, color);
    } else {
        LCD_Paint_WriteLine(lcd, x0, y0, x1, y1, color);
    }
}

void LCD_Paint_DrawCircle(LCD128_HandleTypeDef* lcd, int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;
    LCD_Paint_WritePixel(lcd, x0, y0 + r, color);
    LCD_Paint_WritePixel(lcd, x0, y0 - r, color);
    LCD_Paint_WritePixel(lcd, x0 + r, y0, color);
    LCD_Paint_WritePixel(lcd, x0 - r, y0, color);
    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
        LCD_Paint_WritePixel(lcd, x0 + x, y0 + y, color);
        LCD_Paint_WritePixel(lcd, x0 - x, y0 + y, color);
        LCD_Paint_WritePixel(lcd, x0 + x, y0 - y, color);
        LCD_Paint_WritePixel(lcd, x0 - x, y0 - y, color);
        LCD_Paint_WritePixel(lcd, x0 + y, y0 + x, color);
        LCD_Paint_WritePixel(lcd, x0 - y, y0 + x, color);
        LCD_Paint_WritePixel(lcd, x0 + y, y0 - x, color);
        LCD_Paint_WritePixel(lcd, x0 - y, y0 - x, color);
    }
}

void LCD_Paint_DrawCircleHelper(LCD128_HandleTypeDef* lcd, int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;
    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
        if (cornername & 0x4) {
            LCD_Paint_WritePixel(lcd, x0 + x, y0 + y, color);
            LCD_Paint_WritePixel(lcd, x0 + y, y0 + x, color);
        }
        if (cornername & 0x2) {
            LCD_Paint_WritePixel(lcd, x0 + x, y0 - y, color);
            LCD_Paint_WritePixel(lcd, x0 + y, y0 - x, color);
        }
        if (cornername & 0x8) {
            LCD_Paint_WritePixel(lcd, x0 - y, y0 + x, color);
            LCD_Paint_WritePixel(lcd, x0 - x, y0 + y, color);
        }
        if (cornername & 0x1) {
            LCD_Paint_WritePixel(lcd, x0 - y, y0 - x, color);
            LCD_Paint_WritePixel(lcd, x0 - x, y0 - y, color);
        }
    }
}

void LCD_Paint_FillCircleHelper(LCD128_HandleTypeDef* lcd, int16_t x0, int16_t y0, int16_t r, uint8_t corners, int16_t delta, uint16_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;
    int16_t px = x;
    int16_t py = y;
    delta++;
    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
        if (x < (y + 1)) {
            if (corners & 1) LCD_Paint_DrawFastVLine(lcd, x0 + x, y0 - y, 2 * y + delta, color);
            if (corners & 2) LCD_Paint_DrawFastVLine(lcd, x0 - x, y0 - y, 2 * y + delta, color);
        }
        if (y != py) {
            if (corners & 1) LCD_Paint_DrawFastVLine(lcd, x0 + py, y0 - px, 2 * px + delta, color);
            if (corners & 2) LCD_Paint_DrawFastVLine(lcd, x0 - py, y0 - px, 2 * px + delta, color);
            py = y;
        }
        px = x;
    }
}

void LCD_Paint_FillCircle(LCD128_HandleTypeDef* lcd, int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    LCD_Paint_DrawFastVLine(lcd, x0, y0 - r, 2 * r + 1, color);
    LCD_Paint_FillCircleHelper(lcd, x0, y0, r, 3, 0, color);
}

void LCD_Paint_DrawRect(LCD128_HandleTypeDef* lcd, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    LCD_Paint_DrawFastHLine(lcd, x, y, w, color);
    LCD_Paint_DrawFastHLine(lcd, x, y + h - 1, w, color);
    LCD_Paint_DrawFastVLine(lcd, x, y, h, color);
    LCD_Paint_DrawFastVLine(lcd, x + w - 1, y, h, color);
}

void LCD_Paint_DrawRoundRect(LCD128_HandleTypeDef* lcd, int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    int16_t max_radius = ((w < h) ? w : h) / 2;
    if (r > max_radius) r = max_radius;
    LCD_Paint_DrawFastHLine(lcd, x + r, y, w - 2 * r, color);
    LCD_Paint_DrawFastHLine(lcd, x + r, y + h - 1, w - 2 * r, color);
    LCD_Paint_DrawFastVLine(lcd, x, y + r, h - 2 * r, color);
    LCD_Paint_DrawFastVLine(lcd, x + w - 1, y + r, h - 2 * r, color);
    LCD_Paint_DrawCircleHelper(lcd, x + r, y + r, r, 1, color);
    LCD_Paint_DrawCircleHelper(lcd, x + w - r - 1, y + r, r, 2, color);
    LCD_Paint_DrawCircleHelper(lcd, x + w - r - 1, y + h - r - 1, r, 4, color);
    LCD_Paint_DrawCircleHelper(lcd, x + r, y + h - r - 1, r, 8, color);
}

void LCD_Paint_FillRoundRect(LCD128_HandleTypeDef* lcd, int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    int16_t max_radius = ((w < h) ? w : h) / 2;
    if (r > max_radius) r = max_radius;
    LCD_Paint_FillRect(lcd, x + r, y, w - 2 * r, h, color);
    LCD_Paint_FillCircleHelper(lcd, x + w - r - 1, y + r, r, 1, h - 2 * r - 1, color);
    LCD_Paint_FillCircleHelper(lcd, x + r, y + r, r, 2, h - 2 * r - 1, color);
}

void LCD_Paint_DrawTriangle(LCD128_HandleTypeDef* lcd, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
    LCD_Paint_DrawLine(lcd, x0, y0, x1, y1, color);
    LCD_Paint_DrawLine(lcd, x1, y1, x2, y2, color);
    LCD_Paint_DrawLine(lcd, x2, y2, x0, y0, color);
}

void LCD_Paint_FillTriangle(LCD128_HandleTypeDef* lcd, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
    int16_t a, b, y, last;
    if (y0 > y1) { _swap_int16_t(y0, y1); _swap_int16_t(x0, x1); }
    if (y1 > y2) { _swap_int16_t(y2, y1); _swap_int16_t(x2, x1); }
    if (y0 > y1) { _swap_int16_t(y0, y1); _swap_int16_t(x0, x1); }
    if (y0 == y2) {
        a = b = x0;
        if (x1 < a) a = x1; else if (x1 > b) b = x1;
        if (x2 < a) a = x2; else if (x2 > b) b = x2;
        LCD_Paint_DrawFastHLine(lcd, a, y0, b - a + 1, color);
        return;
    }
    int16_t dx01 = x1 - x0, dy01 = y1 - y0, dx02 = x2 - x0, dy02 = y2 - y0, dx12 = x2 - x1, dy12 = y2 - y1;
    int32_t sa = 0, sb = 0;
    if (y1 == y2) last = y1; else last = y1 - 1;
    for (y = y0; y <= last; y++) {
        a = x0 + sa / dy01;
        b = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;
        if (a > b) _swap_int16_t(a, b);
        LCD_Paint_DrawFastHLine(lcd, a, y, b - a + 1, color);
    }
    sa = (int32_t)dx12 * (y - y1);
    sb = (int32_t)dx02 * (y - y0);
    for (; y <= y2; y++) {
        a = x1 + sa / dy12;
        b = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;
        if (a > b) _swap_int16_t(a, b);
        LCD_Paint_DrawFastHLine(lcd, a, y, b - a + 1, color);
    }
}

void LCD_Paint_DrawDashedEllipse(LCD128_HandleTypeDef* lcd, int16_t x0, int16_t y0, int16_t rx, int16_t ry, int dashStep, int dashLength, int dotRadius, uint16_t color) {
    for (int angle = 0; angle < 360; angle += dashStep) {
        for (int i = 0; i < dashLength; i++) {
            float theta = (angle + i) * 3.14159f / 180.0f;
            int x = x0 + rx * cosf(theta);
            int y = y0 + ry * sinf(theta);
            LCD_Paint_FillCircle(lcd, x, y, dotRadius, color);
//            LCD_Paint_FillCircle(lcd, x + 3, y + 3, dotRadius, color);
        }
    }
} 
