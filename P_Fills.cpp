#include "aurora.h"

/////////////////////////////////
// FILL
/////////////////////////////////
void Fill(CHSV color) {
  strips.fill_solid(color);
}

void resetFill() {
  return;
}

/////////////////////////////////
// PULSE
/////////////////////////////////
#define PULSE_SPEED 5
uint8_t pulsePosition = (PIXELS_PER_STRIP / 2);
uint8_t pulseDirection = -1;

void Pulse(CHSV color) {
  if (stripLengthGate) {
    pulsePosition += pulseDirection;
    if ((pulsePosition > ((PIXELS_PER_STRIP / 2) - 1)) || (pulsePosition <= 0)) {
      pulseDirection *= -1;
    }
  }

  for (uint8_t stripIndex = 0; stripIndex < NUMBER_OF_STRIPS; stripIndex++) {
    for (uint8_t pixelIndex = pulsePosition; pixelIndex <= ((PIXELS_PER_STRIP - 1) - pulsePosition); pixelIndex++) {
      strip[stripIndex][pixelIndex] = color;
    }
  }
}

void resetPulse() {
  pulsePosition = (PIXELS_PER_STRIP / 2);
  pulseDirection = -1;
  return;
}

/////////////////////////////////
// XVISION
/////////////////////////////////
#define XVISION_SPEED 6
#define XVISION_STEPS ((PIXELS_PER_STRIP-1) / (NUMBER_OF_STRIPS-1))
struct xVisionPositions {
  uint8_t startPixelIndex;
  uint8_t endPixelIndex;
};
static xVisionPositions xVision[] = {
  { (0 * XVISION_STEPS), (0 * XVISION_STEPS) },
  { (1 * XVISION_STEPS), (1 * XVISION_STEPS) },
  { (2 * XVISION_STEPS), (2 * XVISION_STEPS) },
  { (3 * XVISION_STEPS), (3 * XVISION_STEPS) },
  { (4 * XVISION_STEPS), (4 * XVISION_STEPS) },
};

static uint8_t xVisionDirection = 0;
static uint8_t xVisionStep = 0;

void XVision(CHSV color) {
  if (stripLengthGate) {
    for (uint8_t stripIndex = 0; stripIndex < NUMBER_OF_STRIPS; stripIndex++) {
      uint8_t xVisionStep = (NUMBER_OF_STRIPS - 1) - stripIndex;
      switch (xVisionDirection) {
        case 0:
          xVision[stripIndex].endPixelIndex += (xVisionStep);
          xVision[stripIndex].startPixelIndex -= (NUMBER_OF_STRIPS - 1) - xVisionStep;
          break;
        case 1:
          xVision[stripIndex].endPixelIndex -= (NUMBER_OF_STRIPS - 1) - xVisionStep;
          xVision[stripIndex].startPixelIndex += xVisionStep;
          break;
        case 2:
          xVision[stripIndex].endPixelIndex += (NUMBER_OF_STRIPS - 1) - xVisionStep;
          xVision[stripIndex].startPixelIndex -= xVisionStep;
          break;
        case 3:
          xVision[stripIndex].endPixelIndex -= xVisionStep;
          xVision[stripIndex].startPixelIndex += (NUMBER_OF_STRIPS - 1) - xVisionStep;
      }
    }

    bool stripEmpty = (xVision[0].endPixelIndex == xVision[0].startPixelIndex);
    bool stripFull = ((xVision[0].endPixelIndex - xVision[0].startPixelIndex) == (PIXELS_PER_STRIP - 1));

    if (stripEmpty || stripFull) {
      xVisionDirection += 1;
      if (xVisionDirection > 3) {
        xVisionDirection = 0;
      }
    }
  }

  for (uint8_t stripIndex = 0; stripIndex < NUMBER_OF_STRIPS; stripIndex++) {
    for (uint8_t pixelIndex = xVision[stripIndex].startPixelIndex; pixelIndex <= xVision[stripIndex].endPixelIndex; pixelIndex++) {
      strip[stripIndex][pixelIndex] = color;
    }
  }
}

void resetXVision() {
  xVision[0] = { (0 * XVISION_STEPS), (0 * XVISION_STEPS) };
  xVision[1] = { (1 * XVISION_STEPS), (1 * XVISION_STEPS) };
  xVision[2] = { (2 * XVISION_STEPS), (2 * XVISION_STEPS) };
  xVision[3] = { (3 * XVISION_STEPS), (3 * XVISION_STEPS) };
  xVision[4] = { (4 * XVISION_STEPS), (4 * XVISION_STEPS) };

  xVisionDirection = 0;
  xVisionStep = 0;
  return;
}
