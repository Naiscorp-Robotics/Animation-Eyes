#ifndef EYE_ANIMATION_H
#define EYE_ANIMATION_H

#include <stdint.h>
#include <stdbool.h>
#include "lcd128.h"

// Enum trạng thái biểu cảm mắt
typedef enum {
    EYE_NORMAL = 0,
    EYE_LOOK_LEFT,
    EYE_LOOK_RIGHT,
    EYE_LOOK_UP,
    EYE_LOOK_DOWN,
    EYE_LOOK_UP_LEFT,
    EYE_LOOK_UP_RIGHT,
    EYE_LOOK_DOWN_LEFT,
    EYE_LOOK_DOWN_RIGHT,
    EYE_BOTH_UP_CROSS,
    EYE_BOTH_DOWN_CROSS,
    EYE_RANDOM_WANDER,
    EYE_SURPRISED,
    EYE_SHAKY,
    EYE_CRYING,
    EYE_HAPPY,
    EYE_SAD,
    EYE_STRETCHED,
    EYE_PINCHED,
    EYE_MULTI_PINCHED,
    EYE_STRAIGHT_SEGMENT,
    EYE_LOWER_HALF,
    EYE_UPPER_HALF,
    EYE_DIZZY,
    EXPRESSION_COUNT
} EyeExpression;

// Cấu hình từng trạng thái mắt
typedef struct {
    uint8_t eyeWidth;
    uint8_t eyeHeight;
    uint8_t pupilSize;
    int8_t pupilOffsetX;
    int8_t pupilOffsetY;
    uint8_t animationType;
} EyeConfig;

// Trạng thái animation hiện tại
typedef struct {
    EyeExpression currentExpression;
    EyeExpression targetExpression;
    uint32_t animationStartTime;
    uint32_t animationDuration;
    bool isAnimating;
    bool isBlinking;
    uint32_t blinkStartTime;
    float breathingPhase;
    uint32_t lastBreathingUpdate;
    bool isWinking;
    bool winkLeftEye;
    uint32_t winkStartTime;
} AnimationState;

// Tham số vẽ mắt
typedef struct {
    int16_t centerX;
    int16_t centerY;
    uint8_t eyeWidth;
    uint8_t eyeHeight;
    int16_t pupilOffsetX;
    int16_t pupilOffsetY;
} EyeDrawParams;

void EyeTFT_Init(LCD128_HandleTypeDef* lcd);
void EyeTFT_Update(void);
void EyeTFT_Draw(LCD128_HandleTypeDef* lcd);
void EyeTFT_SetAutoMode(bool enable);
void EyeTFT_SetExpression(EyeExpression expr, bool smooth);

#endif // EYE_ANIMATION_H 
