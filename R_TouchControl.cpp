#include "aurora.h"

#define NO_TOUCH_POSITION TSPoint()

#define X_AXIS_RESISTANCE 230
#define PIN_TOUCHPAD_YP A4
#define PIN_TOUCHPAD_XM A5
#define PIN_TOUCHPAD_YM 7
#define PIN_TOUCHPAD_XP 6

#define GRID_SIZE_X 5
#define GRID_SIZE_Y 2

#define STEP_SIZE 9

#define X_AXIS_VALUE_LOWER_BOUND 90
#define X_AXIS_VALUE_UPPER_BOUND 930
#define Y_AXIS_VALUE_LOWER_BOUND 220
#define Y_AXIS_VALUE_UPPER_BOUND 860

#define GRID_OFFSET_X ((X_AXIS_VALUE_UPPER_BOUND - X_AXIS_VALUE_LOWER_BOUND) / GRID_SIZE_X) / 2
#define GRID_OFFSET_Y ((Y_AXIS_VALUE_UPPER_BOUND - Y_AXIS_VALUE_LOWER_BOUND) / GRID_SIZE_Y) / 2

static bool holdModeEnabled = false;
static bool presetModeEnabled = false;
static bool variationModeEnabled = false;

static TouchScreen touchScreen = TouchScreen(PIN_TOUCHPAD_XP, PIN_TOUCHPAD_YP, PIN_TOUCHPAD_XM, PIN_TOUCHPAD_YM, X_AXIS_RESISTANCE);
static TSPoint lastTouchPosition = NO_TOUCH_POSITION;
static TSPoint currentTouchPosition = NO_TOUCH_POSITION;
static TSPoint gridPosition = NO_TOUCH_POSITION;

void renderTouchAction() {
  TSPoint touchPosition = currentTouchPosition;
  if ((touchPosition == NO_TOUCH_POSITION) && holdModeEnabled) {
    touchPosition = lastTouchPosition;
  }

  if (touchPosition != NO_TOUCH_POSITION) {
    if (presetModeEnabled) {
      modifyPresetColor(touchPosition);
    } else {
      renderStripColorOverride(touchPosition);
    }
  }
  renderTouchpad();
}

void renderStripColorOverride(TSPoint touchPosition) {
  int16_t yValue = constrain(map(touchPosition.y, Y_AXIS_VALUE_LOWER_BOUND, Y_AXIS_VALUE_UPPER_BOUND, -255, 255), -255, 255);
  uint8_t stripIndex = gridPosition.x;
  if (yValue < 0) {
    touchColor.value = max(0, 255 + yValue);
  } else {
    touchColor.saturation = 255 - yValue;
  }
  if (variationModeEnabled) {
    for (uint8_t pixelIndex = 0; pixelIndex < PIXELS_PER_STRIP; pixelIndex++) {
      if (!strip[stripIndex][pixelIndex] == CRGB::Black) {
        strip[stripIndex][pixelIndex] = blend(strip[stripIndex][pixelIndex], touchColor, 255);
      }
      if (!strip[(NUMBER_OF_STRIPS - 1) - stripIndex][pixelIndex] == CRGB::Black) {
        strip[(NUMBER_OF_STRIPS - 1) - stripIndex][pixelIndex] = blend(strip[stripIndex][pixelIndex], touchColor, 255);
      }
    }
  } else {
    fill_solid(strip[stripIndex], PIXELS_PER_STRIP, touchColor);
    fill_solid(strip[(NUMBER_OF_STRIPS - 1) - stripIndex], PIXELS_PER_STRIP, touchColor);
  }
}

void modifyPresetColor(TSPoint touchPosition) {
  uint8_t xIndex = constrain(map(touchPosition.x, X_AXIS_VALUE_LOWER_BOUND, X_AXIS_VALUE_UPPER_BOUND, -(255 / NUMBER_OF_STRIPS), (255 / NUMBER_OF_STRIPS)), -(255 / NUMBER_OF_STRIPS), (255 / NUMBER_OF_STRIPS));
  int8_t yValue = constrain(map(touchPosition.y, Y_AXIS_VALUE_LOWER_BOUND, Y_AXIS_VALUE_UPPER_BOUND, -128, 127), -128, 127);
  for (uint8_t stripIndex = 0; stripIndex < NUMBER_OF_STRIPS; stripIndex++) {
    uint8_t stripHue = presetColor.hue;
    if (variationModeEnabled) {
      if (stripIndex <= ((NUMBER_OF_STRIPS / 2) )) {
        stripHue += (xIndex * (-(NUMBER_OF_STRIPS / 2) + stripIndex));
      } else {
        stripHue += (xIndex * (-(NUMBER_OF_STRIPS / 2) + ((NUMBER_OF_STRIPS - 1) - stripIndex)));
      }
    } else {
      stripHue += (xIndex * (-(NUMBER_OF_STRIPS / 2) + stripIndex));
    }
    for (uint8_t pixelIndex = 0; pixelIndex < PIXELS_PER_STRIP; pixelIndex++) {
      if (!strip[stripIndex][pixelIndex] == CRGB::Black) {
        int8_t offsetPixelIndex = pixelIndex - (PIXELS_PER_STRIP / 2);
        CHSV pixelColor = CHSV((stripHue + ((offsetPixelIndex * yValue) / PIXELS_PER_STRIP)), presetColor.saturation, presetColor.value);
        strip[stripIndex][pixelIndex] = blend(strip[stripIndex][pixelIndex], pixelColor, 255);
      }
    }
  }
}

void readTouchInputs() {
  holdModeEnabled = (analogRead(PIN_HOLD_MODE) > 511);
  presetModeEnabled = digitalRead(PIN_PRESET_MODE);
  variationModeEnabled = digitalRead(PIN_VARIATION_MODE);
  TSPoint touchPosition = touchScreen.getPoint();
  if (touchPosition.z > touchScreen.pressureThreshhold) {
    currentTouchPosition = touchPosition;
    gridPosition = calculateGridPosition(touchPosition);
    //    colorInvertPositions[gridPosition.x] = constrain(map(touchPosition.y, Y_AXIS_VALUE_LOWER_BOUND, Y_AXIS_VALUE_UPPER_BOUND, 0, ((PIXELS_PER_STRIP - 1) / COLOR_INVERT_STEP_SIZE)), 0, ((PIXELS_PER_STRIP - 1) / COLOR_INVERT_STEP_SIZE));
    //    colorInvertDirection = (((lastTouchPosition.y - touchPosition.y) < 0) ? 1 : -1);
    lastTouchPosition = touchPosition;
  } else {
    currentTouchPosition = NO_TOUCH_POSITION;
  }
}

TSPoint calculateGridPosition(TSPoint touchPosition) {
  uint8_t gridX = constrain(map(touchPosition.x + GRID_OFFSET_X, X_AXIS_VALUE_LOWER_BOUND, X_AXIS_VALUE_UPPER_BOUND, 0, GRID_SIZE_X - 1), 0, GRID_SIZE_X - 1);
  uint8_t gridY = constrain(map(touchPosition.y + (GRID_OFFSET_Y * 2), Y_AXIS_VALUE_LOWER_BOUND, Y_AXIS_VALUE_UPPER_BOUND, 0, GRID_SIZE_Y - 1), 0, GRID_SIZE_Y - 1);

  return TSPoint(gridX, gridY, touchPosition.z);
}

void renderTouchpad() {
  for (uint8_t gridX = 0; gridX < GRID_SIZE_X; gridX++) {
    CRGB upperPixelColor;
    CRGB lowerPixelColor;
    for (uint8_t pixelIndex = 0; pixelIndex < PIXELS_PER_STRIP; pixelIndex++) {
      nblend(upperPixelColor, strip[gridX][(PIXELS_PER_STRIP - 1) - pixelIndex], PIXELS_PER_STRIP);
      nblend(lowerPixelColor, strip[gridX][pixelIndex], PIXELS_PER_STRIP);
    }
    touchpad[gridX] = lowerPixelColor;
    touchpad[(PIXEL_INDEX_TOUCHPAD_END - 1) - gridX] = upperPixelColor;
  }
}

bool isTouched() {
  return currentTouchPosition != NO_TOUCH_POSITION;
}
