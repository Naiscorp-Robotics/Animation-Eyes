#include "eye_animation.h"
#include "lcd128.h"
#include "LCD_Paint.h"
#include "fonts.h"
#include <math.h>
#include <stdlib.h>

#define LCD_WIDTH 240
#define LCD_HEIGHT 240
#define CENTER_X (LCD_WIDTH / 2)
#define CENTER_Y (LCD_HEIGHT / 2)
#define EYE_SPACING 50
#define BASE_EYE_WIDTH 50
#define BASE_EYE_HEIGHT 60
#define BASE_PUPIL_SIZE 28
#define BLINK_DURATION 150
#define EXPRESSION_DURATION 400
#define FRAME_DELAY 25
#define BREATHING_CYCLE 3000
#define constrain(val, min, max) ((val)<(min)?(min):((val)>(max)?(max):(val)))

static EyeConfig eyeConfigs[EXPRESSION_COUNT];
static AnimationState animState;
static uint32_t lastUpdateTime = 0;
static uint32_t lastBlinkTime = 0;
static uint32_t blinkInterval = 3000;
static bool autoMode = true;
static uint32_t lastAutoStateChange = 0;
static uint32_t autoStateInterval = 2500;
static uint8_t autoStateStep = 0;
static uint8_t eyeOutlineStyle = 1;
static int16_t randomWanderTargetX[2] = {0, 0};
static int16_t randomWanderTargetY[2] = {0, 0};
static uint32_t lastRandomWanderUpdate[2] = {0, 0};
static uint32_t surprisedStartTime = 0;
static bool surprisedEffectActive = false;
static float tearY[2] = {0, 0};
static uint32_t lastTearUpdate = 0;

static int16_t getMaxPupilOffsetX(const EyeConfig* config) {
    int16_t maxOffset = 1.8 * (config->eyeWidth / 2) - (config->pupilSize / 2);
    return maxOffset > 0 ? maxOffset : 1;
}
static int16_t getMaxPupilOffsetY(const EyeConfig* config) {
    int16_t maxOffset = 1.8 * (config->eyeWidth / 2) - (config->pupilSize / 2);
    return maxOffset > 0 ? maxOffset : 1;
}

static EyeDrawParams getEyeParams(bool isRightEye, float progress) {
    int16_t baseCenterX = isRightEye ? (CENTER_X + EYE_SPACING) : (CENTER_X - EYE_SPACING);
    int16_t baseCenterY = CENTER_Y;
    uint8_t baseWidth = BASE_EYE_WIDTH;
    uint8_t baseHeight = BASE_EYE_HEIGHT;
    EyeConfig baseConfig = {baseWidth, baseHeight, BASE_PUPIL_SIZE, 0, 0, 0};
    EyeExpression fromExpr = animState.currentExpression;
    EyeExpression toExpr = animState.targetExpression;
    int16_t maxOffsetX = getMaxPupilOffsetX(&baseConfig);
    int16_t maxOffsetY = getMaxPupilOffsetY(&baseConfig);
    if (animState.currentExpression == EYE_SHAKY && !animState.isAnimating) {
        float shakeY = sinf(HAL_GetTick() * 0.04f) * 3 + sinf(HAL_GetTick() * 0.13f) * 2;
        float shakeX = sinf(HAL_GetTick() * 0.07f) * 1.5f;
        return (EyeDrawParams){baseCenterX, baseCenterY, baseWidth, baseHeight, (int16_t)shakeX, (int16_t)shakeY};
    }
    if (animState.currentExpression == EYE_DIZZY && !animState.isAnimating) {
        float t = HAL_GetTick() * 0.007f;
        int16_t r = maxOffsetX * 0.7f;
        int16_t px = cosf(t) * r;
        int16_t py = sinf(t) * r;
        return (EyeDrawParams){baseCenterX, baseCenterY, baseWidth, baseHeight, px, py};
    }
    if (animState.currentExpression == EYE_RANDOM_WANDER && !animState.isAnimating) {
        int eyeIdx = isRightEye ? 1 : 0;
        uint32_t now = HAL_GetTick();
        if (now - lastRandomWanderUpdate[eyeIdx] > (rand() % 400 + 400)) {
            randomWanderTargetX[eyeIdx] = (rand() % (2 * maxOffsetX + 1)) - maxOffsetX;
            randomWanderTargetY[eyeIdx] = (rand() % (2 * maxOffsetY + 1)) - maxOffsetY;
            lastRandomWanderUpdate[eyeIdx] = now;
        }
        static int16_t currentX[2] = {0, 0};
        static int16_t currentY[2] = {0, 0};
        currentX[eyeIdx] += (randomWanderTargetX[eyeIdx] - currentX[eyeIdx]) * 0.15f;
        currentY[eyeIdx] += (randomWanderTargetY[eyeIdx] - currentY[eyeIdx]) * 0.15f;
        return (EyeDrawParams){baseCenterX, baseCenterY, baseWidth, baseHeight, currentX[eyeIdx], currentY[eyeIdx]};
    }
    int16_t fromPupilX = 0, toPupilX = 0;
    int16_t fromPupilY = 0, toPupilY = 0;
    if (fromExpr == EYE_BOTH_UP_CROSS) {
        fromPupilY = -maxOffsetY;
        fromPupilX = isRightEye ? -maxOffsetX : maxOffsetX;
    }
    if (toExpr == EYE_BOTH_UP_CROSS) {
        toPupilY = -maxOffsetY;
        toPupilX = isRightEye ? -maxOffsetX : maxOffsetX;
    }
    if (fromExpr == EYE_BOTH_DOWN_CROSS) {
        fromPupilY = maxOffsetY;
        fromPupilX = isRightEye ? -maxOffsetX : maxOffsetX;
    }
    if (toExpr == EYE_BOTH_DOWN_CROSS) {
        toPupilY = maxOffsetY;
        toPupilX = isRightEye ? -maxOffsetX : maxOffsetX;
    }
    if (fromExpr == EYE_LOOK_LEFT) fromPupilX = -maxOffsetX;
    else if (fromExpr == EYE_LOOK_RIGHT) fromPupilX = maxOffsetX;
    else if (fromExpr == EYE_LOOK_UP_LEFT) fromPupilX = -maxOffsetX;
    else if (fromExpr == EYE_LOOK_UP_RIGHT) fromPupilX = maxOffsetX;
    else if (fromExpr == EYE_LOOK_DOWN_LEFT) fromPupilX = -maxOffsetX;
    else if (fromExpr == EYE_LOOK_DOWN_RIGHT) fromPupilX = maxOffsetX;
    if (toExpr == EYE_LOOK_LEFT) toPupilX = -maxOffsetX;
    else if (toExpr == EYE_LOOK_RIGHT) toPupilX = maxOffsetX;
    else if (toExpr == EYE_LOOK_UP_LEFT) toPupilX = -maxOffsetX;
    else if (toExpr == EYE_LOOK_UP_RIGHT) toPupilX = maxOffsetX;
    else if (toExpr == EYE_LOOK_DOWN_LEFT) toPupilX = -maxOffsetX;
    else if (toExpr == EYE_LOOK_DOWN_RIGHT) toPupilX = maxOffsetX;
    if (fromExpr == EYE_LOOK_UP) fromPupilY = -maxOffsetY;
    if (toExpr == EYE_LOOK_UP) toPupilY = -maxOffsetY;
    if (fromExpr == EYE_LOOK_DOWN) fromPupilY = maxOffsetY;
    if (toExpr == EYE_LOOK_DOWN) toPupilY = maxOffsetY;
    if (fromExpr == EYE_LOOK_UP_LEFT || fromExpr == EYE_LOOK_UP_RIGHT) fromPupilY = -maxOffsetY;
    if (toExpr == EYE_LOOK_UP_LEFT || toExpr == EYE_LOOK_UP_RIGHT) toPupilY = -maxOffsetY;
    if (fromExpr == EYE_LOOK_DOWN_LEFT || fromExpr == EYE_LOOK_DOWN_RIGHT) fromPupilY = maxOffsetY;
    if (toExpr == EYE_LOOK_DOWN_LEFT || toExpr == EYE_LOOK_DOWN_RIGHT) toPupilY = maxOffsetY;
    float t = progress;
    float eased = t < 0.5f ? 4 * t * t * t : 1 - powf(-2 * t + 2, 3) / 2;
    int16_t pupilOffsetX = (int16_t)(fromPupilX + (toPupilX - fromPupilX) * eased + 0.5f);
    int16_t pupilOffsetY = (int16_t)(fromPupilY + (toPupilY - fromPupilY) * eased + 0.5f);
    if (!animState.isAnimating) {
        pupilOffsetX += sinf(HAL_GetTick() / 120.0f) * 0.5f;
        pupilOffsetY += sinf(HAL_GetTick() / 150.0f) * 0.2f;
    }
    return (EyeDrawParams){baseCenterX, baseCenterY, baseWidth, baseHeight, pupilOffsetX, pupilOffsetY};
}

static void initializeEyeConfigs() {
    for (int i = 0; i < EXPRESSION_COUNT; ++i) {
        eyeConfigs[i] = (EyeConfig){BASE_EYE_WIDTH, BASE_EYE_HEIGHT, BASE_PUPIL_SIZE, 0, 0, 0};
    }
    eyeConfigs[EYE_HAPPY].pupilSize = 0;
    eyeConfigs[EYE_SAD].pupilSize = 0;
}

static void setExpression(EyeExpression expression, bool smooth) {
    if (expression >= EXPRESSION_COUNT) return;
    animState.targetExpression = expression;
    if (smooth && expression != animState.currentExpression) {
        animState.isAnimating = true;
        animState.animationStartTime = HAL_GetTick();
        animState.animationDuration = EXPRESSION_DURATION;
    } else {
        animState.currentExpression = expression;
        animState.isAnimating = false;
    }
    if (expression == EYE_SURPRISED) {
        surprisedStartTime = HAL_GetTick();
        surprisedEffectActive = true;
    }
}

static void updateAnimations() {
    uint32_t currentTime = HAL_GetTick();
    if (animState.isBlinking) {
        if ((currentTime - animState.blinkStartTime) > BLINK_DURATION) {
            animState.isBlinking = false;
        }
    }
    if (animState.isWinking) {
        if ((currentTime - animState.winkStartTime) > BLINK_DURATION) {
            animState.isWinking = false;
        }
    }
    if (animState.isAnimating) {
        uint32_t elapsed = currentTime - animState.animationStartTime;
        if (elapsed >= animState.animationDuration) {
            animState.isAnimating = false;
            animState.currentExpression = animState.targetExpression;
        }
    }
}

static void updateBreathing() {
    uint32_t currentTime = HAL_GetTick();
    uint32_t elapsed = currentTime - animState.lastBreathingUpdate;
    if (elapsed >= 50) {
        animState.breathingPhase += (float)elapsed / BREATHING_CYCLE * 2.0f * 3.14159f;
        if (animState.breathingPhase >= 2.0f * 3.14159f) {
            animState.breathingPhase -= 2.0f * 3.14159f;
        }
        animState.lastBreathingUpdate = currentTime;
    }
}

static void triggerBlink() {
    if (!animState.isBlinking && !animState.isWinking) {
        animState.isBlinking = true;
        animState.blinkStartTime = HAL_GetTick();
    }
}
static void triggerWink(bool leftEye) {
    if (!animState.isBlinking && !animState.isWinking) {
        animState.isWinking = true;
        animState.winkLeftEye = leftEye;
        animState.winkStartTime = HAL_GetTick();
    }
}

static float getExpressionProgress() {
    if (animState.isAnimating) {
        uint32_t elapsed = HAL_GetTick() - animState.animationStartTime;
        float progress = (float)elapsed / animState.animationDuration;
        return constrain(progress, 0.0f, 1.0f);
    }
    return 0.0f;
}

// Hàm vẽ elip nét đứt
static void drawDashedEllipse(LCD128_HandleTypeDef* lcd, int x0, int y0, int rx, int ry, int dashStep, int dashLength, int dotRadius) {
    for (int angle = 0; angle < 360; angle += dashStep) {
        for (int i = 0; i < dashLength; i++) {
            float theta = (angle + i) * 3.14159f / 180.0f;
            int x = x0 + rx * cosf(theta);
            int y = y0 + ry * sinf(theta);
            LCD_Paint_FillCircle(lcd, x, y, dotRadius, LCD128_WHITE);
        }
    }
}

// Hàm tổng quát vẽ elip với đoạn thẳng ở cung bất kỳ, tự động đối xứng nếu cần
static void drawEllipseWithStraightSegmentGeneralSym(
    LCD128_HandleTypeDef* lcd,
    int x0, int y0, int rx, int ry,
    int baseStartDeg, int baseEndDeg,
    int dashStep, int dashLength, int dotRadius,
    uint8_t isRightEye,
    uint8_t eyesSymmetric
) {
    int startDeg, endDeg;
    if (eyesSymmetric == 1) {
        startDeg = baseStartDeg;
        endDeg = baseEndDeg;
    } else {
        if (isRightEye) {
            startDeg = 45;
            endDeg = 180;
        } else {
            startDeg = baseStartDeg;
            endDeg = baseEndDeg;
        }
    }
    float thetaStart = startDeg * 3.14159f / 180.0f;
    float thetaEnd = endDeg * 3.14159f / 180.0f;
    int xStart = x0 + rx * cosf(thetaStart);
    int yStart = y0 + ry * sinf(thetaStart);
    int xEnd = x0 + rx * cosf(thetaEnd);
    int yEnd = y0 + ry * sinf(thetaEnd);
    int totalDeg = (endDeg >= startDeg) ? (endDeg - startDeg) : (360 - startDeg + endDeg);
    for (int angle = 0; angle < 360; angle += dashStep) {
        for (int i = 0; i < dashLength; i++) {
            float deg = (float)(angle + i);
            float theta = deg * 3.14159f / 180.0f;
            int x, y;
            bool inSegment = false;
            if (startDeg <= endDeg) {
                inSegment = (deg >= startDeg && deg <= endDeg);
            } else {
                inSegment = (deg >= startDeg || deg <= endDeg);
            }
            if (inSegment) {
                float t = (startDeg <= endDeg)
                    ? (deg - startDeg) / (float)totalDeg
                    : ((deg >= startDeg) ? (deg - startDeg) : (360 - startDeg + deg)) / (float)totalDeg;
                x = xStart + t * (xEnd - xStart);
                y = yStart + t * (yEnd - yStart);
            } else {
                x = x0 + rx * cosf(theta);
                y = y0 + ry * sinf(theta);
            }
            LCD_Paint_FillCircle(lcd, x, y, dotRadius, LCD128_WHITE);
        }
    }
}

// Hàm vẽ viền mắt
static void drawEyeOutline(LCD128_HandleTypeDef* lcd, int16_t centerX, int16_t centerY, uint8_t width, uint8_t height, uint8_t thickness, uint8_t isRightEye) {
    if (thickness == 1) {
        uint8_t dashStep = 15;
        uint8_t dashLength = 2;
        uint8_t dotRadius = 1;
        if (width > 35) {
            dashStep = 10;
            dashLength = 2;
        } else if (width < 25) {
            dashStep = 15;
            dashLength = 1;
        }
        if (animState.currentExpression == EYE_LOWER_HALF) {
            int baseStartDeg = 20;
            int baseEndDeg = 160;
            uint8_t eyesSymmetric = 1;
            drawEllipseWithStraightSegmentGeneralSym(lcd, centerX, centerY, width, height, baseStartDeg, baseEndDeg, dashStep, dashLength, dotRadius, isRightEye, eyesSymmetric);
            drawEllipseWithStraightSegmentGeneralSym(lcd, centerX, centerY, width + 3, height + 3, baseStartDeg, baseEndDeg, dashStep, dashLength, dotRadius, isRightEye, eyesSymmetric);
        } else if (animState.currentExpression == EYE_UPPER_HALF) {
            int baseStartDeg = 200;
            int baseEndDeg = 340;
            uint8_t eyesSymmetric = 1;
            drawEllipseWithStraightSegmentGeneralSym(lcd, centerX, centerY, width, height, baseStartDeg, baseEndDeg, dashStep, dashLength, dotRadius, isRightEye, eyesSymmetric);
            drawEllipseWithStraightSegmentGeneralSym(lcd, centerX, centerY, width + 3, height + 3, baseStartDeg, baseEndDeg, dashStep, dashLength, dotRadius, isRightEye, eyesSymmetric);
        } else if (animState.currentExpression == EYE_STRAIGHT_SEGMENT) {
            int baseStartDeg = 0;
            int baseEndDeg = 135;
            uint8_t eyesSymmetric = 0;
            drawEllipseWithStraightSegmentGeneralSym(lcd, centerX, centerY, width, height, baseStartDeg, baseEndDeg, dashStep, dashLength, dotRadius, isRightEye, eyesSymmetric);
            drawEllipseWithStraightSegmentGeneralSym(lcd, centerX, centerY, width + 3, height + 3, baseStartDeg, baseEndDeg, dashStep, dashLength, dotRadius, isRightEye, eyesSymmetric);
        } else {
            drawDashedEllipse(lcd, centerX, centerY, width, height, dashStep, dashLength, dotRadius);
            drawDashedEllipse(lcd, centerX, centerY, width + 3, height + 3, dashStep, dashLength, dotRadius);
        }
    }
}

// Hàm vẽ đồng tử
static void drawPupil(LCD128_HandleTypeDef* lcd, int16_t eyeCenterX, int16_t eyeCenterY, const EyeConfig* config, uint8_t isRightEye) {
    int16_t pupilX = eyeCenterX + config->pupilOffsetX;
    int16_t pupilY = eyeCenterY + config->pupilOffsetY;
    uint8_t pupilR = config->pupilSize / 2;
    int16_t maxOffsetX = getMaxPupilOffsetX(config);
    int16_t maxOffsetY = getMaxPupilOffsetY(config);
    pupilX = constrain(pupilX, eyeCenterX - maxOffsetX, eyeCenterX + maxOffsetX);
    pupilY = constrain(pupilY, eyeCenterY - maxOffsetY, eyeCenterY + maxOffsetY);
    LCD_Paint_FillCircle(lcd, pupilX, pupilY, pupilR, LCD128_WHITE);
    float pupilOffsetRatioX = 0, pupilOffsetRatioY = 0;
    if (maxOffsetX != 0) pupilOffsetRatioX = (float)config->pupilOffsetX / maxOffsetX;
    if (maxOffsetY != 0) pupilOffsetRatioY = (float)config->pupilOffsetY / maxOffsetY;
    int16_t innerMaxOffset = pupilR - 2;
    int16_t innerOffsetX = pupilOffsetRatioX * innerMaxOffset;
    int16_t innerOffsetY = pupilOffsetRatioY * innerMaxOffset;
    if (config->pupilSize >= 8) {
        LCD_Paint_FillCircle(lcd, pupilX + innerOffsetX, pupilY + innerOffsetY, pupilR / 2, LCD128_BLACK);
        if (config->pupilSize >= 12) {
            LCD128_DrawPixel(lcd, pupilX + innerOffsetX - 1, pupilY + innerOffsetY - 1, LCD128_WHITE);
        }
    }
}

static void drawHappyEye(LCD128_HandleTypeDef* lcd, int centerX, int centerY, bool isRightEye) {
    if (isRightEye) {
        int rightV[3][4] = {
            {centerX - 10, centerY, centerX, centerY - 5},
            {centerX - 11, centerY, centerX - 1, centerY - 5},
            {centerX - 9,  centerY, centerX + 1, centerY - 5},
        };
        int rightV2[3][4] = {
            {centerX - 10, centerY, centerX + 4, centerY + 15},
            {centerX - 11, centerY, centerX + 3, centerY + 15},
            {centerX - 9,  centerY, centerX + 5, centerY + 15},
        };
        for (int i = 0; i < 3; i++) {
            LCD_Paint_WriteLine(lcd, rightV[i][0], rightV[i][1], rightV[i][2], rightV[i][3], LCD128_WHITE);
            LCD_Paint_WriteLine(lcd, rightV2[i][0], rightV2[i][1], rightV2[i][2], rightV2[i][3], LCD128_WHITE);
        }
    } else {
        int leftV[3][4] = {
            {centerX + 10, centerY, centerX, centerY - 5},
            {centerX + 11, centerY, centerX + 1, centerY - 5},
            {centerX + 9,  centerY, centerX - 1, centerY - 5},
        };
        int leftV2[3][4] = {
            {centerX + 10, centerY, centerX - 4, centerY + 15},
            {centerX + 11, centerY, centerX - 3, centerY + 15},
            {centerX + 9,  centerY, centerX - 5, centerY + 15},
        };
        for (int i = 0; i < 3; i++) {
            LCD_Paint_WriteLine(lcd, leftV[i][0], leftV[i][1], leftV[i][2], leftV[i][3], LCD128_WHITE);
            LCD_Paint_WriteLine(lcd, leftV2[i][0], leftV2[i][1], leftV2[i][2], leftV2[i][3], LCD128_WHITE);
        }
    }
}

static void drawSadEye(LCD128_HandleTypeDef* lcd, int centerX, int centerY, bool isRightEye) {
    int radius = 15;
    if (isRightEye) {
        for (int r = radius; r <= radius + 1; r++){
            for (float angle = 100; angle <= 200; angle += 2) {
                float rad = angle * 3.14159f / 180.0f;
                int px = centerX + (int)(r * cosf(rad)) + 4;
                int py = centerY + (int)(r * sinf(rad)) - 2;
                LCD128_DrawPixel(lcd, px, py, LCD128_WHITE);
            }
        }
    } else {
        for (int r = radius; r <= radius + 1; r++) {
            for (float angle = -20; angle <= 80; angle += 2) {
                float rad = angle * 3.14159f / 180.0f;
                int px = centerX + (int)(r * cosf(rad)) - 4;
                int py = centerY + (int)(r * sinf(rad)) - 2;
                LCD128_DrawPixel(lcd, px, py, LCD128_WHITE);
            }
        }
    }
}

// Hàm vẽ mắt
static void drawEye(LCD128_HandleTypeDef* lcd, int16_t centerX, int16_t centerY, const EyeConfig* config, uint8_t isRightEye, float progress) {
    uint8_t eyeWidth = config->eyeWidth;
    uint8_t eyeHeight = config->eyeHeight;
    EyeConfig drawConfig = *config;
    if (animState.currentExpression == EYE_SURPRISED && surprisedEffectActive) {
        uint32_t now = HAL_GetTick();
        uint32_t elapsed = now - surprisedStartTime;
        uint8_t minPupil = BASE_PUPIL_SIZE * 0.7f;
        uint8_t maxPupil = BASE_PUPIL_SIZE * 1.5f;
        if (elapsed < 200) {
            float t = elapsed / 200.0f;
            drawConfig.pupilSize = minPupil + (maxPupil - minPupil) * t;
        } else if (elapsed < 500) {
            float t = (elapsed - 200) / 300.0f;
            drawConfig.pupilSize = maxPupil - (maxPupil - BASE_PUPIL_SIZE) * t;
        } else {
            drawConfig.pupilSize = BASE_PUPIL_SIZE;
            surprisedEffectActive = false;
        }
    }
    drawEyeOutline(lcd, centerX, centerY, eyeWidth, eyeHeight, eyeOutlineStyle, isRightEye);
    if (animState.currentExpression == EYE_HAPPY) {
        drawHappyEye(lcd, centerX, centerY, isRightEye);
    } else if (animState.currentExpression == EYE_SAD) {
        drawSadEye(lcd, centerX, centerY, isRightEye);
    } else if (drawConfig.pupilSize > 0) {
        drawPupil(lcd, centerX, centerY, &drawConfig, isRightEye);
        int16_t reflectX = centerX + drawConfig.pupilOffsetX - 2;
        int16_t reflectY = centerY + drawConfig.pupilOffsetY - 1;
        LCD128_DrawPixel(lcd, reflectX, reflectY, LCD128_BLACK);
    }
}

void EyeTFT_Init(LCD128_HandleTypeDef* lcd) {
    initializeEyeConfigs();
    animState.currentExpression = EYE_NORMAL;
    animState.targetExpression = EYE_NORMAL;
    animState.isAnimating = false;
    animState.isBlinking = false;
    animState.isWinking = false;
    animState.breathingPhase = 0.0f;
    animState.lastBreathingUpdate = HAL_GetTick();
    lastUpdateTime = HAL_GetTick();
    lastBlinkTime = HAL_GetTick();
    blinkInterval = 3000;
    autoMode = true;
    lastAutoStateChange = HAL_GetTick();
    autoStateInterval = 2500;
    autoStateStep = 0;
    eyeOutlineStyle = 1;
    surprisedEffectActive = false;
    tearY[0] = tearY[1] = 0;
    lastTearUpdate = HAL_GetTick();
    drawDashedEllipse(lcd, CENTER_X, CENTER_Y, 20, 40, 15, 2, 1); // vẽ trực tiếp lên LCD
}

void EyeTFT_Update(void) {
    uint32_t currentTime = HAL_GetTick();
    if (autoMode && (currentTime - lastAutoStateChange > autoStateInterval)) {
        autoStateStep = (autoStateStep + 1) % 25;
        switch (autoStateStep) {
            case 0:
            case 2:
            case 4:
            case 6:
            case 23:
                setExpression(EYE_NORMAL, true);
                break;
            case 1:
                setExpression(EYE_LOOK_LEFT, true);
                break;
            case 3:
                setExpression(EYE_LOOK_RIGHT, true);
                break;
            case 5:
                setExpression(EYE_LOOK_UP, true);
                break;
            case 7:
                setExpression(EYE_LOOK_DOWN, true);
                break;
            case 8:
                setExpression(EYE_LOOK_UP_LEFT, true);
                break;
            case 9:
                setExpression(EYE_LOOK_UP_RIGHT, true);
                break;
            case 10:
                setExpression(EYE_LOOK_DOWN_LEFT, true);
                break;
            case 11:
                setExpression(EYE_LOOK_DOWN_RIGHT, true);
                break;
            case 12:
                setExpression(EYE_BOTH_UP_CROSS, true);
                break;
            case 13:
                setExpression(EYE_BOTH_DOWN_CROSS, true);
                break;
            case 14:
                setExpression(EYE_RANDOM_WANDER, true);
                break;
            case 15:
                setExpression(EYE_SURPRISED, false);
                break;
            case 16:
                setExpression(EYE_SHAKY, false);
                break;
            case 17:
                setExpression(EYE_CRYING, false);
                break;
            case 18:
                setExpression(EYE_DIZZY, false);
                break;
            case 19:
                setExpression(EYE_STRAIGHT_SEGMENT, true);
                break;
            case 20:
                setExpression(EYE_UPPER_HALF, true);
                break;
            case 21:
                setExpression(EYE_LOWER_HALF, true);
                break;
            case 22:
                setExpression(EYE_SAD, true);
                break;
            case 24:
                setExpression(EYE_HAPPY, true);
                break;
        }
        lastAutoStateChange = currentTime;
        autoStateInterval = (rand() % 1200) + 1800;
    }
    if (autoMode && !animState.isBlinking && !animState.isWinking && (currentTime - lastBlinkTime) > blinkInterval) {
        if ((rand() % 10) < 2) {
            triggerWink(rand() % 2);
        } else {
            triggerBlink();
        }
        lastBlinkTime = currentTime;
        blinkInterval = (rand() % 4000) + 2000;
    }
    updateAnimations();
    updateBreathing();
    if (animState.currentExpression == EYE_CRYING) {
        if (currentTime - lastTearUpdate > 30) {
            for (int i = 0; i < 2; ++i) {
                tearY[i] += 1.5f;
                if (tearY[i] > 18) tearY[i] = 0;
            }
            lastTearUpdate = currentTime;
        }
    }
}

void EyeTFT_Draw(LCD128_HandleTypeDef* lcd) {
    LCD128_FillScreen(lcd, LCD128_BLACK);
    float progress = getExpressionProgress();
    float breathingEffect = sinf(animState.breathingPhase) * 0.3f;
    EyeDrawParams left = getEyeParams(false, progress);
    EyeConfig leftConfig = eyeConfigs[animState.currentExpression];
    leftConfig.eyeWidth = left.eyeWidth;
    leftConfig.eyeHeight = left.eyeHeight + breathingEffect;
    leftConfig.pupilOffsetX = left.pupilOffsetX;
    leftConfig.pupilOffsetY = left.pupilOffsetY;
    bool leftWink = animState.isWinking && animState.winkLeftEye;
    if (leftWink) {
        EyeConfig winkConfig = leftConfig;
        winkConfig.eyeHeight = 2;
        winkConfig.pupilSize = 0;
        drawEye(lcd, left.centerX, left.centerY, &winkConfig, false, progress);
    } else {
        drawEye(lcd, left.centerX, left.centerY, &leftConfig, false, progress);
    }
    EyeDrawParams right = getEyeParams(true, progress);
    EyeConfig rightConfig = eyeConfigs[animState.currentExpression];
    rightConfig.eyeWidth = right.eyeWidth;
    rightConfig.eyeHeight = right.eyeHeight + breathingEffect;
    rightConfig.pupilOffsetX = right.pupilOffsetX;
    rightConfig.pupilOffsetY = right.pupilOffsetY;
    bool rightWink = animState.isWinking && !animState.winkLeftEye;
    if (rightWink) {
        EyeConfig winkConfig = rightConfig;
        winkConfig.eyeHeight = 2;
        winkConfig.pupilSize = 0;
        drawEye(lcd, right.centerX, right.centerY, &winkConfig, true, progress);
    } else {
        drawEye(lcd, right.centerX, right.centerY, &rightConfig, true, progress);
    }
    if (animState.currentExpression == EYE_CRYING) {
        int xL = CENTER_X - EYE_SPACING;
        int yL = CENTER_Y + BASE_EYE_HEIGHT / 2 + (int)tearY[0];
        LCD_Paint_FillCircle(lcd, xL, yL, 4, LCD128_WHITE);
        int xR = CENTER_X + EYE_SPACING;
        int yR = CENTER_Y + BASE_EYE_HEIGHT / 2 + (int)tearY[1];
        LCD_Paint_FillCircle(lcd, xR, yR, 4, LCD128_WHITE);
    }
}

void EyeTFT_SetAutoMode(bool enable) {
    autoMode = enable;
}

void EyeTFT_SetExpression(EyeExpression expr, bool smooth) {
    setExpression(expr, smooth);
}
