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
  STATE_DOWNRIGHT_FROM_CENTER,
  STATE_UPLEFT_TO_CENTER,

  STATE_RANDOM_MOVE,
  STATE_RANDOM_MOVE1,
  STATE_RANDOM_TO_CENTER,
  STATE_SURPRISED_GROW,
  STATE_SURPRISED_SHRINK,
  STATE_PUPIL_ROTATE

} MoveState;

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

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
extern volatile uint8_t lcd128_dma_busy;
int centerX, centerY;
int rx_in, ry_in;
int pupilRadius, eyeRadius;
int pupilX, pupilY;
MoveState state;
uint32_t last_tick;
uint32_t frame_count;
uint32_t fps;
char fps_buf[16];
int lerp_startX, lerp_startY, lerp_targetX, lerp_targetY;
float lerp_t;
int lerp_steps;
int random_count;
const int RANDOM_REPEAT = 20;

// // Thêm khai báo extern biến trạng thái DMA
//   extern volatile uint8_t lcd128_dma_busy;
//   // Khởi tạo tham số.
//   int centerX = 120, centerY = 120;
//   int rx_in  = 70, ry_in  = 90;   // Bán trục lớn của elip trong
//   int pupilRadius = 32;
//   int eyeRadius = pupilRadius / 2;
//   int pupilX = centerX, pupilY = centerY;
//   /* USER CODE END 2 */
//   MoveState state = STATE_RANDOM_MOVE1;
//   // Thêm biến đo FPS
//   uint32_t last_tick = HAL_GetTick();
//   uint32_t frame_count = 0;
//   uint32_t fps = 0;
//   char fps_buf[16];
//   // Thêm biến cho nội suy
//   int lerp_startX = 0, lerp_startY = 0, lerp_targetX = 0, lerp_targetY = 0;
//   float lerp_t = 1.0f; // 1.0 nghĩa là đã đến đích, cần setup mới
//   int lerp_steps = 40; // Số frame để di chuyển (có thể điều chỉnh tốc độ)
//   static int random_count = 0;
//   const int RANDOM_REPEAT = 30; // Số lần random liên tiếp mong muốn

// Thứ tự các trạng thái chuyển động
MoveState state_sequence[] = {
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
    STATE_DOWNLEFT_FROM_CENTER,
    STATE_UPRIGHT_TO_CENTER,
    STATE_DOWNRIGHT_FROM_CENTER,
    STATE_UPLEFT_TO_CENTER,

    STATE_RANDOM_MOVE,
    STATE_RANDOM_MOVE1,
    STATE_RANDOM_TO_CENTER,
};
int state_sequence_len;
int state_index;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI2_Init(void);

/* USER CODE BEGIN PFP */
#define BUF_W 70
#define BUF_H 70
uint16_t framebuf[BUF_H][BUF_W];
void draw_eye_with_pupil_to_buffer(int cx, int cy, int r, int pupil_r, int pupil_offset_x, int pupil_offset_y, uint16_t eye_color, uint16_t pupil_color, uint16_t bgcolor);
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi);
void Animation_Loop(void);
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

  LCD128_FillScreen(&lcd1, LCD128_BLACK);
  LCD128_FillScreen(&lcd2, LCD128_BLACK);

  LCD_Paint_DrawDashedEllipse(&lcd1, 120, 120, 83, 108, 6, 2, 2, LCD128_WHITE);
  LCD_Paint_DrawDashedEllipse(&lcd1, 120, 120, 90, 115, 7, 2, 2, LCD128_WHITE);
  LCD_Paint_DrawDashedEllipse(&lcd1, 120, 120, 95, 120, 8, 2, 2, LCD128_WHITE);
  LCD_Paint_DrawDashedEllipse(&lcd2, 120, 120, 83, 108, 6, 2, 2, LCD128_WHITE);
  LCD_Paint_DrawDashedEllipse(&lcd2, 120, 120, 90, 115, 7, 2, 2, LCD128_WHITE);
  LCD_Paint_DrawDashedEllipse(&lcd2, 120, 120, 95, 120, 8, 2, 2, LCD128_WHITE);



  centerX = 120; centerY = 120;
  rx_in  = 70; ry_in  = 90;
  pupilRadius = 32;
  eyeRadius = pupilRadius / 2;
  pupilX = centerX; pupilY = centerY; 
  state = STATE_RANDOM_MOVE1;
  last_tick = HAL_GetTick();
  frame_count = 0;
  fps = 0;
  lerp_startX = lerp_startY = lerp_targetX = lerp_targetY = 0;
  lerp_t = 1.0f;
  lerp_steps = 30;
  random_count = 0;
  state_sequence_len = sizeof(state_sequence) / sizeof(state_sequence[0]);
  state_index = 0;
  state = state_sequence[state_index];

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      Animation_Loop();
	    HAL_Delay(10);
      frame_count++;
      uint32_t now = HAL_GetTick();
      if (now - last_tick >= 1000) { 
        fps = frame_count;
        frame_count = 0;
        last_tick = now;
        sprintf(fps_buf, "%lu", fps);
        LCD_Paint_FillRect(&lcd1, 0, 120, 60, 16, LCD128_BLACK); 
        LCD128_WriteString(&lcd1, 0, 120, fps_buf, Font_11x18, LCD128_WHITE, LCD128_BLACK);
      }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

      draw_eye_with_pupil_to_buffer(BUF_W/2, BUF_H/2, pupilRadius, eyeRadius, pupil_offset_x, pupil_offset_y, LCD128_WHITE, LCD128_BLACK, LCD128_BLACK);

	    LCD128_DrawImage_DMA(&lcd1, pupilX - BUF_W/2, pupilY - BUF_H/2, BUF_W, BUF_H, (uint16_t*)framebuf);
	    LCD128_DrawImage_DMA(&lcd2, pupilX - BUF_W/2, pupilY - BUF_H/2, BUF_W, BUF_H, (uint16_t*)framebuf);
	    while (lcd128_dma_busy);

      if (lerp_t < 1.0f) {
          lerp_t += 1.0f / lerp_steps;
          if (lerp_t > 1.0f) lerp_t = 1.0f;
          pupilX = lerp_startX + (int)((lerp_targetX - lerp_startX) * lerp_t);
          pupilY = lerp_startY + (int)((lerp_targetY - lerp_startY) * lerp_t);
      } else {
          switch (state) {
                case STATE_RANDOM_MOVE1: {
                      int rangeX = rx_in - pupilRadius * 2;
                      int rangeY = ry_in - pupilRadius * 2;
                  
                      if (random_count < RANDOM_REPEAT) {
                          lerp_startX = pupilX;
                          lerp_startY = pupilY;
                  
                          // Tạo offset đối xứng quanh tâm
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
                  lerp_targetX = centerX - (rx_in - pupilRadius) * 0.7f;
                  lerp_targetY = centerY - (ry_in - pupilRadius) * 0.7f;
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
              case STATE_UPRIGHT_FROM_CENTER:
                  lerp_startX = pupilX; lerp_startY = pupilY;
                  lerp_targetX = centerX + (rx_in - pupilRadius) * 0.7f;
                  lerp_targetY = centerY - (ry_in - pupilRadius) * 0.7f;
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
                  lerp_targetX = centerX - (rx_in - pupilRadius) * 0.7f;
                  lerp_targetY = centerY + (ry_in - pupilRadius) * 0.7f;
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
              case STATE_DOWNRIGHT_FROM_CENTER:
                  lerp_startX = pupilX; lerp_startY = pupilY;
                  lerp_targetX = centerX + (rx_in - pupilRadius) * 0.7f;
                  lerp_targetY = centerY + (ry_in - pupilRadius) * 0.7f;
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
              // Random move
              case STATE_RANDOM_MOVE:
                  if (random_count < RANDOM_REPEAT) {
                      lerp_startX = pupilX; lerp_startY = pupilY;
                      lerp_targetX = centerX + (rand() % (rx_in - pupilRadius * 2)) - (rx_in - pupilRadius) / 2;
                      lerp_targetY = centerY + (rand() % (ry_in - pupilRadius * 2)) - (ry_in - pupilRadius) / 2;
                      lerp_t = 0.0f;
                      random_count++;
                  } else {
                      random_count = 0;
                      lerp_startX = pupilX; lerp_startY = pupilY;
                      lerp_targetX = centerX;
                      lerp_targetY = centerY;
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
              case STATE_SURPRISED_GROW:
              case STATE_SURPRISED_SHRINK:
                  break;
              
          }
      }
}

void draw_eye_with_pupil_to_buffer(int cx, int cy, int r, int pupil_r, int pupil_offset_x, int pupil_offset_y, uint16_t eye_color, uint16_t pupil_color, uint16_t bgcolor) {
    // Vẽ tròng mắt (vòng tròn trắng)
    for (int y = 0; y < BUF_H; y++) {
        for (int x = 0; x < BUF_W; x++) {
            int dx = x - cx;
            int dy = y - cy;
            if (dx*dx + dy*dy <= r*r)
                framebuf[y][x] = eye_color;   // Màu tròng mắt
            else
                framebuf[y][x] = bgcolor;     // Màu nền
        }
    }
    // Vẽ con ngươi (vòng tròn đen nhỏ hơn, có thể lệch tâm)
    int pupil_cx = cx + pupil_offset_x;
    int pupil_cy = cy + pupil_offset_y;
    for (int y = 0; y < BUF_H; y++) {
        for (int x = 0; x < BUF_W; x++) {
            int dx = x - pupil_cx;
            int dy = y - pupil_cy;
            if (dx*dx + dy*dy <= pupil_r*pupil_r)
                framebuf[y][x] = LCD128_YELLOW; // Màu con ngươi
        }
    }
  }

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
    if (hspi->Instance == SPI1) {
        // Đóng CS sau khi DMA xong
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET); // CS_Pin
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
