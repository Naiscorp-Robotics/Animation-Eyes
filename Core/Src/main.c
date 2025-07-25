/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "lcd128.h"
#include "math.h"
#include "LCD_Paint.h"
#include <stdlib.h>
#include <stdio.h>
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
typedef enum {
  STATE_UP_FROM_CENTER,
  STATE_DOWN_TO_CENTER,
  STATE_LEFT_FROM_CENTER,
  STATE_RIGHT_TO_CENTER,
  STATE_DOWN_FROM_CENTER,
  STATE_UP_TO_CENTER,
  STATE_RIGHT_FROM_CENTER,
  STATE_LEFT_TO_CENTER,

  STATE_UPLEFT_FROM_CENTER,
  STATE_DOWNRIGHT_TO_CENTER,
  STATE_UPRIGHT_FROM_CENTER,
  STATE_DOWNLEFT_TO_CENTER,
  STATE_DOWNLEFT_FROM_CENTER,
  STATE_UPRIGHT_TO_CENTER,
//  STATE_DOWNRIGHT_FROM_CENTER,
  STATE_UPLEFT_TO_CENTER,

  STATE_RANDOM_MOVE,
  STATE_RANDOM_MOVE1,
  STATE_RANDOM_TO_CENTER,
  STATE_PUPIL_ROTATE

} MoveState;

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define BUF_W 120
#define BUF_H 120
#define BLOCK_LINES 1
void draw_eye_with_pupil_to_buffer(int cx, int cy, int r, int pupil_r, int pupil_offset_x, int pupil_offset_y, uint16_t outer_color, uint16_t inner_color, uint16_t bgcolor, uint16_t pupil_color);
void draw_realistic_eye(int cx, int cy, int r, int pupil_r, int pupil_offset_x, int pupil_offset_y,uint16_t bgcolor); 
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi);
void Animation_Loop(void);
void update_animation_state(void);
void draw_eye_line_with_pupil_to_buffer(
  int y, int cx, int cy, int r,
  int pupil_r, int pupil_offset_x, int pupil_offset_y,
  uint16_t outer_color, uint16_t inner_color, uint16_t bgcolor, uint16_t pupil_color,
  uint16_t* linebuf,
  int highlight_tick
);
// === Blend nhanh giữa 2 màu RGB565 ===
uint16_t blend_color_fast(uint16_t c1, uint16_t c2, uint8_t t) {
    uint8_t r1 = (c1 >> 11) & 0x1F, g1 = (c1 >> 5) & 0x3F, b1 = c1 & 0x1F;
    uint8_t r2 = (c2 >> 11) & 0x1F, g2 = (c2 >> 5) & 0x3F, b2 = c2 & 0x1F;

    uint8_t r = ((r1 * (255 - t)) + (r2 * t)) >> 8;
    uint8_t g = ((g1 * (255 - t)) + (g2 * t)) >> 8;
    uint8_t b = ((b1 * (255 - t)) + (b2 * t)) >> 8;

    return (r << 11) | (g << 5) | b;
}

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;
DMA_HandleTypeDef hdma_spi1_tx;
DMA_HandleTypeDef hdma_spi2_tx;
LCD128_HandleTypeDef lcd1;
LCD128_HandleTypeDef lcd2;
/* USER CODE BEGIN PV */
// Trong USER CODE BEGIN PV
// Thêm khai báo extern biến trạng thái DMA
extern volatile uint8_t lcd128_dma_busy;
uint8_t centerX, centerY;
uint8_t rx_in, ry_in; // Bán trục lớn của elip trong
uint8_t pupilRadius, eyeRadius;
uint8_t pupilX, pupilY;
MoveState state;
// Thêm biến cho nội suy
int lerp_startX, lerp_startY, lerp_targetX, lerp_targetY;
float lerp_t;
int lerp_steps; // Số frame để di chuyển (có thể điều chỉnh tốc độ)
const int RANDOM_REPEAT = 20; // Số lần random liên tiếp mong muốn
uint8_t state_index, state_sequence_len, random_count;
// Thêm biến toàn cục cho highlight_tick
int global_highlight_tick = 0;
// Thứ tự các trạng thái chuyển động
const MoveState state_sequence[] = {
    STATE_RANDOM_MOVE1,
    STATE_UP_FROM_CENTER,
    STATE_UPLEFT_FROM_CENTER,
    STATE_UPRIGHT_FROM_CENTER,
    STATE_DOWN_TO_CENTER,
    STATE_RANDOM_MOVE1,
    STATE_LEFT_FROM_CENTER,
    STATE_RIGHT_TO_CENTER,
    STATE_DOWN_FROM_CENTER,
    STATE_UPLEFT_TO_CENTER,
    STATE_UP_TO_CENTER,
    STATE_DOWNLEFT_FROM_CENTER,
    STATE_RIGHT_FROM_CENTER,
    STATE_LEFT_TO_CENTER,

    STATE_UPLEFT_FROM_CENTER,
    STATE_DOWNRIGHT_TO_CENTER,
    STATE_UPRIGHT_TO_CENTER,
    STATE_UPRIGHT_FROM_CENTER,
    STATE_DOWNLEFT_TO_CENTER,
    STATE_RANDOM_MOVE1,
    STATE_DOWNLEFT_FROM_CENTER,
    STATE_UPRIGHT_TO_CENTER,
//    STATE_DOWNRIGHT_FROM_CENTER,
    STATE_UPLEFT_TO_CENTER,
    STATE_RANDOM_MOVE,
    STATE_RANDOM_TO_CENTER,
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  /* USER CODE BEGIN 2 */
  lcd1.hspi = &hspi1;
  lcd1.CS_Port = GPIOB; lcd1.CS_Pin = GPIO_PIN_10;
  lcd1.DC_Port = GPIOB; lcd1.DC_Pin = GPIO_PIN_1;
  lcd1.RST_Port = GPIOB; lcd1.RST_Pin = GPIO_PIN_0;


  lcd2.hspi = &hspi2;
  lcd2.CS_Port = GPIOA; lcd2.CS_Pin = GPIO_PIN_10;
  lcd2.DC_Port = GPIOA; lcd2.DC_Pin = GPIO_PIN_9;
  lcd2.RST_Port = GPIOA; lcd2.RST_Pin = GPIO_PIN_8;

  LCD128_Init(&lcd1);
  LCD128_Init(&lcd2);

  LCD128_FillScreen(&lcd1, LCD128_WHITE);
  LCD128_FillScreen(&lcd2, LCD128_WHITE);

  LCD_Paint_DrawDashedEllipse(&lcd1, 120, 120, 105, 105, 10, 7, 2, LCD128_BLACK);
  LCD_Paint_DrawDashedEllipse(&lcd1, 120, 120, 112, 112, 10, 7, 2, LCD128_BLACK);
  LCD_Paint_DrawDashedEllipse(&lcd1, 120, 120, 118, 118, 10, 7, 2, LCD128_BLACK);

  LCD_Paint_DrawDashedEllipse(&lcd2, 120, 120, 105, 105, 10, 7, 2, LCD128_BLACK);
  LCD_Paint_DrawDashedEllipse(&lcd2, 120, 120, 112, 112, 10, 7, 2, LCD128_BLACK);
  LCD_Paint_DrawDashedEllipse(&lcd2, 120, 120, 118, 118, 10, 7, 2, LCD128_BLACK);


  centerX = 120; centerY = 120;
  rx_in  = 65; ry_in  = 65;
  pupilRadius = 55;
  eyeRadius = pupilRadius / 2;
  pupilX = centerX; pupilY = centerY; 
  lerp_startX = lerp_startY = lerp_targetX = lerp_targetY = 0;
  lerp_t = 1.0f;
  lerp_steps = 10;
  random_count = 0;
  state_sequence_len = sizeof(state_sequence) / sizeof(state_sequence[0]);
  state_index = 0;
  state = state_sequence[state_index];

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      // --- Cập nhật trạng thái animation ---
      update_animation_state();
      // --- Khóa trạng thái, vẽ và gửi frame ---
      Animation_Loop();
      // HAL_Delay(2);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void Animation_Loop(void){

      // Tính offset pupil theo hướng di chuyển, giới hạn trong mắt trắng
      int pupil_offset_x = centerX - pupilX;  
      int pupil_offset_y = centerY - pupilY;

      float max_offset = eyeRadius - pupilRadius;
      float dist = sqrtf(pupil_offset_x * pupil_offset_x + pupil_offset_y * pupil_offset_y);
      if (dist > max_offset && dist > 0) {
          pupil_offset_x = (int)(pupil_offset_x * max_offset / dist);
          pupil_offset_y = (int)(pupil_offset_y * max_offset / dist);
      }
      // --- KHÓA TRẠNG THÁI ---
      int pupilX_snapshot = pupilX;
      int pupilY_snapshot = pupilY;
      int pupilRadius_snapshot = pupilRadius;
      int eyeRadius_snapshot = eyeRadius;
      int pupil_offset_x_snapshot = pupil_offset_x;
      int pupil_offset_y_snapshot = pupil_offset_y;

      // Gửi từng hàng, KHÔNG cần framebuf lớn
      uint16_t linebuf[BUF_W * BLOCK_LINES];
      int highlight_tick_snapshot = global_highlight_tick;
      for (int y = 0; y < BUF_H; y += BLOCK_LINES) {
          int lines_to_send = (y + BLOCK_LINES <= BUF_H) ? BLOCK_LINES : (BUF_H - y);
          for (int i = 0; i < lines_to_send; i++) {
              draw_eye_line_with_pupil_to_buffer(y + i, BUF_W/2, BUF_H/2, pupilRadius_snapshot, eyeRadius_snapshot, pupil_offset_x_snapshot, pupil_offset_y_snapshot,
                                                 LCD128_BLACK, EYE_BLUE_1, LCD128_WHITE, LCD128_BLACK,
                                                 &linebuf[i * BUF_W],
                                                 highlight_tick_snapshot);
          }
          LCD128_DrawImage_DMA(&lcd1, pupilX_snapshot - BUF_W/2, pupilY_snapshot - BUF_H/2 + y, BUF_W, lines_to_send, linebuf);
          while (lcd128_dma_busy);
          LCD128_DrawImage_DMA(&lcd2, pupilX_snapshot - BUF_W/2, pupilY_snapshot - BUF_H/2 + y, BUF_W, lines_to_send, linebuf);
          while (lcd128_dma_busy);
      }

      // --- PAUSE LOGIC ---
      // static int pause_counter = 0; // Moved to update_animation_state
      // static int is_pausing = 0; // Moved to update_animation_state
      // int is_random_state = (state == STATE_RANDOM_MOVE || state == STATE_RANDOM_MOVE1 || state == STATE_RANDOM_TO_CENTER); // Moved to update_animation_state
      // if (lerp_t >= 1.0f) { // Moved to update_animation_state
      //     if (!is_random_state) { // Chỉ pause nếu KHÔNG phải random
      //         if (!is_pausing) {
      //             pause_counter = 0;
      //             is_pausing = 1;
      //         }
      //         if (pause_counter < 10) { // 40 frame ~ 0.7s
      //             pause_counter++;
      //             return;
      //         }
      //         is_pausing = 0;
      //     }
      //     // Sau khi pause (hoặc nếu là random), cho phép chuyển trạng thái mới
      // }

      // if (lerp_t < 1.0f) { // Moved to update_animation_state
      //     lerp_t += 1.5f / lerp_steps;
      //     if (lerp_t > 1.0f) lerp_t = 1.0f;
      //     pupilX = lerp_startX + (int)((lerp_targetX - lerp_startX) * lerp_t);
      //     pupilY = lerp_startY + (int)((lerp_targetY - lerp_startY) * lerp_t);
      // } else if (!is_pausing) { // Moved to update_animation_state
      //     switch (state) { // Moved to update_animation_state
      //           case STATE_RANDOM_MOVE1: { // Moved to update_animation_state
      //               int rangeX = rx_in - pupilRadius * 0.5; // Moved to update_animation_state
      //               int rangeY = ry_in - pupilRadius * 0.5; // Moved to update_animation_state
      //               if (random_count < RANDOM_REPEAT) { // Moved to update_animation_state
      //                   lerp_startX = pupilX; // Moved to update_animation_state
      //                   lerp_startY = pupilY; // Moved to update_animation_state
      //                   int offsetX = (rand() % rangeX) - rangeX / 2; // Moved to update_animation_state
      //                   int offsetY = (rand() % rangeY) - rangeY / 2; // Moved to update_animation_state
      //                   lerp_targetX = centerX + offsetX; // Moved to update_animation_state
      //                   lerp_targetY = centerY + offsetY; // Moved to update_animation_state
      //                   lerp_t = 0.0f; // Moved to update_animation_state
      //                   random_count++; // Moved to update_animation_state
      //               } else { // Moved to update_animation_state
      //                   random_count = 0; // Moved to update_animation_state
      //                   lerp_startX = pupilX; // Moved to update_animation_state
      //                   lerp_startY = pupilY; // Moved to update_animation_state
      //                   lerp_targetX = centerX; // Moved to update_animation_state
      //                   lerp_targetY = centerY; // Moved to update_animation_state
      //                   lerp_t = 0.0f; // Moved to update_animation_state
      //                   state_index++; // Moved to update_animation_state
      //                   if (state_index >= state_sequence_len) state_index = 0; // Moved to update_animation_state
      //                   state = state_sequence[state_index]; // Moved to update_animation_state
      //               } // Moved to update_animation_state
      //               break; // Moved to update_animation_state
      //           } // Moved to update_animation_state
      //       case STATE_UP_FROM_CENTER: // Moved to update_animation_state
      //           lerp_startX = pupilX; lerp_startY = pupilY; // Moved to update_animation_state
      //           lerp_targetX = centerX; // Moved to update_animation_state
      //           lerp_targetY = centerY - (ry_in - pupilRadius); // Moved to update_animation_state
      //           lerp_t = 0.0f; // Moved to update_animation_state
      //           state_index++; // Moved to update_animation_state
      //           if (state_index >= state_sequence_len) state_index = 0; // Moved to update_animation_state
      //           state = state_sequence[state_index]; // Moved to update_animation_state
      //           break; // Moved to update_animation_state
      //       case STATE_DOWN_TO_CENTER: // Moved to update_animation_state
      //           lerp_startX = pupilX; lerp_startY = pupilY; // Moved to update_animation_state
      //           lerp_targetX = centerX; // Moved to update_animation_state
      //           lerp_targetY = centerY; // Moved to update_animation_state
      //           lerp_t = 0.0f; // Moved to update_animation_state
      //           state_index++; // Moved to update_animation_state
      //           if (state_index >= state_sequence_len) state_index = 0; // Moved to update_animation_state
      //           state = state_sequence[state_index]; // Moved to update_animation_state
      //           break; // Moved to update_animation_state
      //       case STATE_LEFT_FROM_CENTER: // Moved to update_animation_state
      //           lerp_startX = pupilX; lerp_startY = pupilY; // Moved to update_animation_state
      //           lerp_targetX = centerX - (rx_in - pupilRadius); // Moved to update_animation_state
      //           lerp_targetY = centerY; // Moved to update_animation_state
      //           lerp_t = 0.0f; // Moved to update_animation_state
      //           state_index++; // Moved to update_animation_state
      //           if (state_index >= state_sequence_len) state_index = 0; // Moved to update_animation_state
      //           state = state_sequence[state_index]; // Moved to update_animation_state
      //           break; // Moved to update_animation_state
      //       case STATE_RIGHT_TO_CENTER: // Moved to update_animation_state
      //           lerp_startX = pupilX; lerp_startY = pupilY; // Moved to update_animation_state
      //           lerp_targetX = centerX; // Moved to update_animation_state
      //           lerp_targetY = centerY; // Moved to update_animation_state
      //           lerp_t = 0.0f; // Moved to update_animation_state
      //           state_index++; // Moved to update_animation_state
      //           if (state_index >= state_sequence_len) state_index = 0; // Moved to update_animation_state
      //           state = state_sequence[state_index]; // Moved to update_animation_state
      //           break; // Moved to update_animation_state
      //       case STATE_DOWN_FROM_CENTER: // Moved to update_animation_state
      //           lerp_startX = pupilX; lerp_startY = pupilY; // Moved to update_animation_state
      //           lerp_targetX = centerX; // Moved to update_animation_state
      //           lerp_targetY = centerY + (ry_in - pupilRadius); // Moved to update_animation_state
      //           lerp_t = 0.0f; // Moved to update_animation_state
      //           state_index++; // Moved to update_animation_state
      //           if (state_index >= state_sequence_len) state_index = 0; // Moved to update_animation_state
      //           state = state_sequence[state_index]; // Moved to update_animation_state
      //           break; // Moved to update_animation_state
      //       case STATE_UP_TO_CENTER: // Moved to update_animation_state
      //           lerp_startX = pupilX; lerp_startY = pupilY; // Moved to update_animation_state
      //           lerp_targetX = centerX; // Moved to update_animation_state
      //           lerp_targetY = centerY; // Moved to update_animation_state
      //           lerp_t = 0.0f; // Moved to update_animation_state
      //           state_index++; // Moved to update_animation_state
      //           if (state_index >= state_sequence_len) state_index = 0; // Moved to update_animation_state
      //           state = state_sequence[state_index]; // Moved to update_animation_state
      //           break; // Moved to update_animation_state
      //       case STATE_RIGHT_FROM_CENTER: // Moved to update_animation_state
      //           lerp_startX = pupilX; lerp_startY = pupilY; // Moved to update_animation_state
      //           lerp_targetX = centerX + (rx_in - pupilRadius); // Moved to update_animation_state
      //           lerp_targetY = centerY; // Moved to update_animation_state
      //           lerp_t = 0.0f; // Moved to update_animation_state
      //           state_index++; // Moved to update_animation_state
      //           if (state_index >= state_sequence_len) state_index = 0; // Moved to update_animation_state
      //           state = state_sequence[state_index]; // Moved to update_animation_state
      //           break; // Moved to update_animation_state
      //       case STATE_LEFT_TO_CENTER: // Moved to update_animation_state
      //           lerp_startX = pupilX; lerp_startY = pupilY; // Moved to update_animation_state
      //           lerp_targetX = centerX; // Moved to update_animation_state
      //           lerp_targetY = centerY; // Moved to update_animation_state
      //           lerp_t = 0.0f; // Moved to update_animation_state
      //           state_index++; // Moved to update_animation_state
      //           if (state_index >= state_sequence_len) state_index = 0; // Moved to update_animation_state
      //           state = state_sequence[state_index]; // Moved to update_animation_state
      //           break; // Moved to update_animation_state
      //       case STATE_UPLEFT_FROM_CENTER: // Moved to update_animation_state
      //           lerp_startX = pupilX; lerp_startY = pupilY; // Moved to update_animation_state
      //           lerp_targetX = centerX - (rx_in - pupilRadius) * 0.7f; // Moved to update_animation_state
      //           lerp_targetY = centerY - (ry_in - pupilRadius) * 0.7f; // Moved to update_animation_state
      //           lerp_t = 0.0f; // Moved to update_animation_state
      //           state_index++; // Moved to update_animation_state
      //           if (state_index >= state_sequence_len) state_index = 0; // Moved to update_animation_state
      //           state = state_sequence[state_index]; // Moved to update_animation_state
      //           break; // Moved to update_animation_state
      //       case STATE_DOWNRIGHT_TO_CENTER: // Moved to update_animation_state
      //           lerp_startX = pupilX; lerp_startY = pupilY; // Moved to update_animation_state
      //           lerp_targetX = centerX; // Moved to update_animation_state
      //           lerp_targetY = centerY; // Moved to update_animation_state
      //           lerp_t = 0.0f; // Moved to update_animation_state
      //           state_index++; // Moved to update_animation_state
      //           if (state_index >= state_sequence_len) state_index = 0; // Moved to update_animation_state
      //           state = state_sequence[state_index]; // Moved to update_animation_state
      //           break; // Moved to update_animation_state
      //       case STATE_UPRIGHT_FROM_CENTER: // Moved to update_animation_state
      //           lerp_startX = pupilX; lerp_startY = pupilY; // Moved to update_animation_state
      //           lerp_targetX = centerX + (rx_in - pupilRadius) * 0.7f; // Moved to update_animation_state
      //           lerp_targetY = centerY - (ry_in - pupilRadius) * 0.7f; // Moved to update_animation_state
      //           lerp_t = 0.0f; // Moved to update_animation_state
      //           state_index++; // Moved to update_animation_state
      //           if (state_index >= state_sequence_len) state_index = 0; // Moved to update_animation_state
      //           state = state_sequence[state_index]; // Moved to update_animation_state
      //           break; // Moved to update_animation_state
      //       case STATE_DOWNLEFT_TO_CENTER: // Moved to update_animation_state
      //           lerp_startX = pupilX; lerp_startY = pupilY; // Moved to update_animation_state
      //           lerp_targetX = centerX; // Moved to update_animation_state
      //           lerp_targetY = centerY; // Moved to update_animation_state
      //           lerp_t = 0.0f; // Moved to update_animation_state
      //           state_index++; // Moved to update_animation_state
      //           if (state_index >= state_sequence_len) state_index = 0; // Moved to update_animation_state
      //           state = state_sequence[state_index]; // Moved to update_animation_state
      //           break; // Moved to update_animation_state
      //       case STATE_DOWNLEFT_FROM_CENTER: // Moved to update_animation_state
      //           lerp_startX = pupilX; lerp_startY = pupilY; // Moved to update_animation_state
      //           lerp_targetX = centerX - (rx_in - pupilRadius) * 0.7f; // Moved to update_animation_state
      //           lerp_targetY = centerY + (ry_in - pupilRadius) * 0.7f; // Moved to update_animation_state
      //           lerp_t = 0.0f; // Moved to update_animation_state
      //           state_index++; // Moved to update_animation_state
      //           if (state_index >= state_sequence_len) state_index = 0; // Moved to update_animation_state
      //           state = state_sequence[state_index]; // Moved to update_animation_state
      //           break; // Moved to update_animation_state
      //       case STATE_UPRIGHT_TO_CENTER: // Moved to update_animation_state
      //           lerp_startX = pupilX; lerp_startY = pupilY; // Moved to update_animation_state
      //           lerp_targetX = centerX; // Moved to update_animation_state
      //           lerp_targetY = centerY; // Moved to update_animation_state
      //           lerp_t = 0.0f; // Moved to update_animation_state
      //           state_index++; // Moved to update_animation_state
      //           if (state_index >= state_sequence_len) state_index = 0; // Moved to update_animation_state
      //           state = state_sequence[state_index]; // Moved to update_animation_state
      //           break; // Moved to update_animation_state
      //       case STATE_DOWNRIGHT_FROM_CENTER: // Moved to update_animation_state
      //           lerp_startX = pupilX; lerp_startY = pupilY; // Moved to update_animation_state
      //           lerp_targetX = centerX + (rx_in - pupilRadius) * 0.7f; // Moved to update_animation_state
      //           lerp_targetY = centerY + (ry_in - pupilRadius) * 0.7f; // Moved to update_animation_state
      //           lerp_t = 0.0f; // Moved to update_animation_state
      //           state_index++; // Moved to update_animation_state
      //           if (state_index >= state_sequence_len) state_index = 0; // Moved to update_animation_state
      //           state = state_sequence[state_index]; // Moved to update_animation_state
      //           break; // Moved to update_animation_state
      //       case STATE_UPLEFT_TO_CENTER: // Moved to update_animation_state
      //           lerp_startX = pupilX; lerp_startY = pupilY; // Moved to update_animation_state
      //           lerp_targetX = centerX; // Moved to update_animation_state
      //           lerp_targetY = centerY; // Moved to update_animation_state
      //           lerp_t = 0.0f; // Moved to update_animation_state
      //           state_index++; // Moved to update_animation_state
      //           if (state_index >= state_sequence_len) state_index = 0; // Moved to update_animation_state
      //           state = state_sequence[state_index]; // Moved to update_animation_state
      //           break; // Moved to update_animation_state
      //       // Random move // Moved to update_animation_state
      //       case STATE_RANDOM_MOVE: // Moved to update_animation_state
      //           if (random_count < RANDOM_REPEAT) { // Moved to update_animation_state
      //               lerp_startX = pupilX; lerp_startY = pupilY; // Moved to update_animation_state
      //               lerp_targetX = centerX + (rand() % (rx_in - pupilRadius * 2)); // Moved to update_animation_state
      //               lerp_targetY = centerY + (rand() % (ry_in - pupilRadius * 2)); // Moved to update_animation_state
      //               lerp_t = 0.0f; // Moved to update_animation_state
      //               random_count++; // Moved to update_animation_state
      //           } else { // Moved to update_animation_state
      //               random_count = 0; // Moved to update_animation_state
      //               lerp_startX = pupilX; lerp_startY = pupilY; // Moved to update_animation_state
      //               lerp_targetX = centerX; // Moved to update_animation_state
      //               lerp_targetY = centerY; // Moved to update_animation_state
      //               lerp_t = 0.0f; // Moved to update_animation_state
      //               state_index++; // Moved to update_animation_state
      //               if (state_index >= state_sequence_len) state_index = 0; // Moved to update_animation_state
      //               state = state_sequence[state_index]; // Moved to update_animation_state
      //           } // Moved to update_animation_state
      //           break;            // Moved to update_animation_state
      //       case STATE_RANDOM_TO_CENTER: // Moved to update_animation_state
      //           lerp_startX = pupilX; lerp_startY = pupilY; // Moved to update_animation_state
      //           lerp_targetX = centerX; // Moved to update_animation_state
      //           lerp_targetY = centerY; // Moved to update_animation_state
      //           lerp_t = 0.0f; // Moved to update_animation_state
      //           state_index++; // Moved to update_animation_state
      //           if (state_index >= state_sequence_len) state_index = 0; // Moved to update_animation_state
      //           state = state_sequence[state_index]; // Moved to update_animation_state
      //           break; // Moved to update_animation_state
      //       case STATE_PUPIL_ROTATE: // Moved to update_animation_state
      //           break; // Moved to update_animation_state
              
      //   } // Moved to update_animation_state
      // } // Moved to update_animation_state
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

// update_animation_state chỉ cập nhật logic chuyển động, không vẽ/gửi
void update_animation_state(void) {
    global_highlight_tick++;
    static int pause_counter = 0;
    static int is_pausing = 0;
    int is_random_state = (state == STATE_RANDOM_MOVE || state == STATE_RANDOM_MOVE1 || state == STATE_RANDOM_TO_CENTER);
    if (lerp_t >= 1.0f) {
        if (!is_random_state) { // Chỉ pause nếu KHÔNG phải random
            if (!is_pausing) {
                pause_counter = 0;
                is_pausing = 1;
            }
            if (pause_counter < 10) { // 40 frame ~ 0.7s
                pause_counter++;
                return;
            }
            is_pausing = 0;
        }
    }

    if (lerp_t < 1.0f) {
        lerp_t += 1.4f / lerp_steps;
        if (lerp_t > 1.0f) lerp_t = 1.0f;
        pupilX = lerp_startX + (int)((lerp_targetX - lerp_startX) * lerp_t);
        pupilY = lerp_startY + (int)((lerp_targetY - lerp_startY) * lerp_t);
    } else if (!is_pausing) {
        switch (state) {
            case STATE_RANDOM_MOVE1: {
                int rangeX = rx_in - pupilRadius * 0.5;
                int rangeY = ry_in - pupilRadius * 0.5;
                if (random_count < RANDOM_REPEAT) {
                    lerp_startX = pupilX;
                    lerp_startY = pupilY;
                    int offsetX = (rand() % rangeX) - rangeX / 2;
                    int offsetY = (rand() % rangeY) - rangeY / 2;
                    lerp_targetX = centerX + offsetX;
                    lerp_targetY = centerY + offsetY;
                    lerp_t = 0.0f;
                    random_count++;
                } else {
                    random_count = 0;
                    lerp_startX = pupilX;
                    lerp_startY = pupilY;
                    lerp_targetX = centerX;
                    lerp_targetY = centerY;
                    lerp_t = 0.0f;
                    state_index++;
                    if (state_index >= state_sequence_len) state_index = 0;
                    state = state_sequence[state_index];
                }
                break;
            }
            case STATE_UP_FROM_CENTER:
                lerp_startX = pupilX; lerp_startY = pupilY;
                lerp_targetX = centerX;
                lerp_targetY = centerY - (ry_in - pupilRadius);
                lerp_t = 0.0f;
                state_index++;
                if (state_index >= state_sequence_len) state_index = 0;
                state = state_sequence[state_index];
                break;
            case STATE_DOWN_TO_CENTER:
                lerp_startX = pupilX; lerp_startY = pupilY;
                lerp_targetX = centerX;
                lerp_targetY = centerY;
                lerp_t = 0.0f;
                state_index++;
                if (state_index >= state_sequence_len) state_index = 0;
                state = state_sequence[state_index];
                break;
            case STATE_LEFT_FROM_CENTER:
                lerp_startX = pupilX; lerp_startY = pupilY;
                lerp_targetX = centerX - (rx_in - pupilRadius);
                lerp_targetY = centerY;
                lerp_t = 0.0f;
                state_index++;
                if (state_index >= state_sequence_len) state_index = 0;
                state = state_sequence[state_index];
                break;
            case STATE_RIGHT_TO_CENTER:
                lerp_startX = pupilX; lerp_startY = pupilY;
                lerp_targetX = centerX;
                lerp_targetY = centerY;
                lerp_t = 0.0f;
                state_index++;
                if (state_index >= state_sequence_len) state_index = 0;
                state = state_sequence[state_index];
                break;
            case STATE_DOWN_FROM_CENTER:
                lerp_startX = pupilX; lerp_startY = pupilY;
                lerp_targetX = centerX;
                lerp_targetY = centerY + (ry_in - pupilRadius);
                lerp_t = 0.0f;
                state_index++;
                if (state_index >= state_sequence_len) state_index = 0;
                state = state_sequence[state_index];
                break;
            case STATE_UP_TO_CENTER:
                lerp_startX = pupilX; lerp_startY = pupilY;
                lerp_targetX = centerX;
                lerp_targetY = centerY;
                lerp_t = 0.0f;
                state_index++;
                if (state_index >= state_sequence_len) state_index = 0;
                state = state_sequence[state_index];
                break;
            case STATE_RIGHT_FROM_CENTER:
                lerp_startX = pupilX; lerp_startY = pupilY;
                lerp_targetX = centerX + (rx_in - pupilRadius);
                lerp_targetY = centerY;
                lerp_t = 0.0f;
                state_index++;
                if (state_index >= state_sequence_len) state_index = 0;
                state = state_sequence[state_index];
                break;
            case STATE_LEFT_TO_CENTER:
                lerp_startX = pupilX; lerp_startY = pupilY;
                lerp_targetX = centerX;
                lerp_targetY = centerY;
                lerp_t = 0.0f;
                state_index++;
                if (state_index >= state_sequence_len) state_index = 0;
                state = state_sequence[state_index];
                break;
            case STATE_UPLEFT_FROM_CENTER:
                lerp_startX = pupilX; lerp_startY = pupilY;
                lerp_targetX = centerX - (rx_in - pupilRadius) / 1.4;
                lerp_targetY = centerY - (ry_in - pupilRadius) / 1.4;
                lerp_t = 0.0f;
                state_index++;
                if (state_index >= state_sequence_len) state_index = 0;
                state = state_sequence[state_index];
                break;
            case STATE_DOWNRIGHT_TO_CENTER:
                lerp_startX = pupilX; lerp_startY = pupilY;
                lerp_targetX = centerX;
                lerp_targetY = centerY;
                lerp_t = 0.0f;
                state_index++;
                if (state_index >= state_sequence_len) state_index = 0;
                state = state_sequence[state_index];
                break;
            case STATE_UPRIGHT_TO_CENTER:
                lerp_startX = pupilX; lerp_startY = pupilY;
                lerp_targetX = centerX;
                lerp_targetY = centerY;
                lerp_t = 0.0f;
                state_index++;
                if (state_index >= state_sequence_len) state_index = 0;
                state = state_sequence[state_index];
                break;
            case STATE_UPRIGHT_FROM_CENTER:
                lerp_startX = pupilX; lerp_startY = pupilY;
                lerp_targetX = centerX + (rx_in - pupilRadius) / 1.4;
                lerp_targetY = centerY - (ry_in - pupilRadius) / 1.4;
                lerp_t = 0.0f;
                state_index++;
                if (state_index >= state_sequence_len) state_index = 0;
                state = state_sequence[state_index];
                break;
            case STATE_DOWNLEFT_TO_CENTER:
                lerp_startX = pupilX; lerp_startY = pupilY;
                lerp_targetX = centerX;
                lerp_targetY = centerY;
                lerp_t = 0.0f;
                state_index++;
                if (state_index >= state_sequence_len) state_index = 0;
                state = state_sequence[state_index];
                break;
            case STATE_DOWNLEFT_FROM_CENTER:
                lerp_startX = pupilX; lerp_startY = pupilY;
                lerp_targetX = centerX - (rx_in - pupilRadius) / 1.4;
                lerp_targetY = centerY + (ry_in - pupilRadius) / 1.4;
                lerp_t = 0.0f;
                state_index++;
                if (state_index >= state_sequence_len) state_index = 0;
                state = state_sequence[state_index];
                break;
            case STATE_UPLEFT_TO_CENTER:
                lerp_startX = pupilX; lerp_startY = pupilY;
                lerp_targetX = centerX;
                lerp_targetY = centerY;
                lerp_t = 0.0f;
                state_index++;
                if (state_index >= state_sequence_len) state_index = 0;
                state = state_sequence[state_index];
                break;
            case STATE_RANDOM_MOVE:
                {
                    int rangeX = rx_in - pupilRadius * 0.5;
                    int rangeY = ry_in - pupilRadius * 0.5;
                    lerp_startX = pupilX;
                    lerp_startY = pupilY;
                    int offsetX = (rand() % rangeX) - rangeX / 2;
                    int offsetY = (rand() % rangeY) - rangeY / 2;
                    lerp_targetX = centerX + offsetX;
                    lerp_targetY = centerY + offsetY;
                    lerp_t = 0.0f;
                    state_index++;
                    if (state_index >= state_sequence_len) state_index = 0;
                    state = state_sequence[state_index];
                }
                break;
            case STATE_RANDOM_TO_CENTER:
                lerp_startX = pupilX; lerp_startY = pupilY;
                lerp_targetX = centerX;
                lerp_targetY = centerY;
                lerp_t = 0.0f;
                state_index++;
                if (state_index >= state_sequence_len) state_index = 0;
                state = state_sequence[state_index];
                break;
            case STATE_PUPIL_ROTATE:
                break;
        }
    }
}

// Định nghĩa đúng cho draw_eye_line_with_pupil_to_buffer:
void draw_eye_line_with_pupil_to_buffer(
  int y, int cx, int cy, int r,
  int pupil_r, int pupil_offset_x, int pupil_offset_y,
  uint16_t outer_color, uint16_t inner_color, uint16_t bgcolor, uint16_t pupil_color,
  uint16_t* linebuf,
  int highlight_tick
) {
  int r2 = r * r ;
  int pupil_cx = cx;
  int pupil_cy = cy;
  float shake_ampl = 2.0f;
  float shake1 = shake_ampl * sinf(highlight_tick * 0.15f);
  float shake2 = shake_ampl * cosf(highlight_tick * 0.18f);
  int highlight_cx3 = cx - r / 3 + (int)(1.5f * sinf(highlight_tick * 0.22f)) + 35;
  int highlight_cy3 = cy - r / 3 + (int)(1.5f * cosf(highlight_tick * 0.19f)) - 10;
  int show_highlight3 = ((highlight_tick % 120) < 10);
  int dy = y - cy;
  int dy2 = dy * dy;
  for (int x = 0; x < BUF_W; x++) {
      int dx = x - cx;
      int dist2 = dx * dx + dy2;
      uint16_t color = bgcolor;
      if (dist2 <= r2) {
          int t = (dist2 * 255) / r2;
          color = blend_color_fast(inner_color, outer_color, (uint8_t)(t*0.7f));
          int dx_pupil = x - pupil_cx;
          int dy_pupil = y - pupil_cy;
          if (dx_pupil * dx_pupil + dy_pupil * dy_pupil <= pupil_r * pupil_r) {
              color = 0x0000;
          }
      }
      int highlight_radius = 10;
      int highlight_cx = cx - r / 3 + (int)shake1;
      int highlight_cy = cy - r / 3 + (int)shake2;
      int dx_h = x - highlight_cx;
      int dy_h = y - highlight_cy;
      if (dx_h * dx_h + dy_h * dy_h <= highlight_radius * highlight_radius) {
          color = 0xFFFF;
      }
      int highlight_radius1 = 4;
      int highlight_cx1 = 2 * cx - highlight_cx;
      int highlight_cy1 = 2 * cy - highlight_cy;
      int dx_h1 = x - highlight_cx1;
      int dy_h1 = y - highlight_cy1;
      if (dx_h1 * dx_h1 + dy_h1 * dy_h1 <= highlight_radius1 * highlight_radius1) {
          color = 0xFFFF;
      }
      int highlight_radius3 = 8;
      int dx_h3 = x - highlight_cx3;
      int dy_h3 = y - highlight_cy3;
      if (show_highlight3 && (dx_h3 * dx_h3 + dy_h3 * dy_h3 <= highlight_radius3 * highlight_radius3)) {
          color = 0xFFFF;
      }
      linebuf[x] = color;
  }
}


void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
    if (hspi->Instance == SPI1) {
        // Đóng CS sau khi DMA xong cho LCD1
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET); // CS_Pin LCD1
        lcd128_dma_busy = 0;
    }
    if (hspi->Instance == SPI2) {
        // Đóng CS sau khi DMA xong cho LCD2
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET); // CS_Pin LCD2
        lcd128_dma_busy = 0;
    }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_10, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10, GPIO_PIN_RESET);

  /*Configure GPIO pins : PB0 PB1 PB10 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PA8 PA9 PA10 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */
