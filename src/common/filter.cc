#include "filter.h"

#include <math.h>

#if __STDC_HOSTED__ != 1
#include "errno.h"
#endif
#ifdef __ARM_NEON
// disable warnings from arm_neon internals only
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnarrowing"

// #include <arm_neon.h>

#pragma GCC diagnostic pop
#define USE_NEON_SQRT 1
#else
#define USE_NEON_SQRT 0
#endif
#include <stdint.h>
#include <sys/types.h>

// iterative simple maths
void com::Filter::grayscale(uint8_t *rgbData, int32_t nPixelBits,
                            uint8_t *newRgb) {
  for (int32_t i = 0; i < nPixelBits; i += 4) {
    // simple avrage to get the grey
    uint8_t gray = static_cast<uint8_t>(
        (rgbData[i] + rgbData[i + 1] + rgbData[i + 2]) / 3);

    newRgb[i] = gray;     // Red
    newRgb[i + 1] = gray; // Green
    newRgb[i + 2] = gray; // Blue
  }
}

// convolution
void com::Filter::sobelEdgeDetect(uint8_t *rgbData, int32_t nPixelBits,
                                  int32_t frameStride, uint8_t *newRgb) {
  // sobel kernel
  int32_t gX[3][3] = {{1, 0, -1}, {2, 0, -2}, {1, 0, -1}};
  int32_t gY[3][3] = {{1, 2, 1}, {0, 0, 0}, {-1, -2, -1}};

  int32_t maxCols = frameStride / 4 - 1;
  for (int32_t i = 0; i < nPixelBits; i += 4) {
    // deal with top and bottom row
    uint8_t *abv =
        (i < frameStride) ? (&rgbData[i]) : (&rgbData[i - frameStride]);
    uint8_t *blw = (i > nPixelBits - frameStride) ? (&rgbData[i])
                                                  : (&rgbData[i + frameStride]);
    uint8_t *kernalOnRgb[3][3] = {
        {abv - 4, abv, abv + 4},
        {(&rgbData[i]) - 4, (&rgbData[i]), (&rgbData[i]) + 4},
        {blw - 4, blw, blw + 4}};
    // deal with l and r edge

    int32_t col = (i % frameStride) / 4;
    if (col == 0) {
      kernalOnRgb[0][0] = abv;
      kernalOnRgb[1][0] = (&rgbData[i]);
      kernalOnRgb[2][0] = blw;
    }
    if (col == maxCols) {
      kernalOnRgb[0][2] = abv;
      kernalOnRgb[1][2] = (&rgbData[i]);
      kernalOnRgb[2][2] = blw;
    }
    int32_t resX = 0;
    int32_t resY = 0;
    float sobelValF = 0;
    for (int32_t j = 0; j < 3; j++) {
      for (int32_t k = 0; k < 3; k++) {
        resX += gX[j][k] * (*kernalOnRgb[j][k]);
        resY += gY[j][k] * (*kernalOnRgb[j][k]);
      }
    }
    float toSqrt = static_cast<float>((resX * resX) + (resY * resY));
// to make it runnable for testing on non-arm computers
#if USE_NEON_SQRT && OP_FLAG
#pragma message("neon square root")
    // extremely slow when not on O3
    __asm__ volatile("fsqrt %s0, %s1" : "=w"(sobelValF) : "w"(toSqrt));
#else
    sobelValF = sqrtf(toSqrt);
#endif

    int32_t sobelVal = static_cast<int32_t>(roundf(sobelValF));
    if (sobelVal > 255) {
      sobelVal = 255;
    };

    newRgb[i] = sobelVal;
    newRgb[i + 1] = sobelVal;
    newRgb[i + 2] = sobelVal;
    newRgb[i + 3] = 255; // full opacity
  }
}

void com::Filter::fisheyeTransform(uint8_t *rgbData, int32_t nPixelBits,
                                   int32_t frameStride, uint8_t *newRgb,
                                   float strength) {
  int32_t centerX = frameStride / 8;
  int32_t centerY = (nPixelBits / frameStride) / 2;
  // fit based off the smaller axis
  int32_t szDeterminer = 0;
  if (centerX < centerY) {
    szDeterminer = centerX;
  } else {
    szDeterminer = centerY;
  }

  for (int32_t i = 0; i < nPixelBits; i += 4) {
    int32_t col = (i % frameStride) / 4;
    int32_t row = i / frameStride;

    // euclidian distance from center
    int32_t dx = col - centerX;
    int32_t dy = row - centerY;
    float r = 0;
    float toSqrt = static_cast<float>(dx * dx + dy * dy);
#if USE_NEON_SQRT && OP_FLAG
    // extremely slow when not on O3
    __asm__ volatile("fsqrt %s0, %s1" : "=w"(r) : "w"(toSqrt));
#else
    r = sqrtf(toSqrt);
#endif

    // apply strength of fisheye distortion based off dist from center
    float dist = r / szDeterminer;
    dist = powf(dist, strength);
    float scaleX = (dist * dx) / r;
    float scaleY = (dist * dy) / r;

    // distorted pixel position in image
    int32_t newCol = static_cast<int32_t>(centerX + scaleX * r);
    int32_t newRow = static_cast<int32_t>(centerY + scaleY * r);

    // ensure pixel is inside image
    if (newCol >= 0 && newCol < frameStride / 4 && newRow >= 0 &&
        newRow < nPixelBits / frameStride) {
      int32_t newIndex = (newRow * frameStride) + (newCol * 4);
      newRgb[i] = rgbData[newIndex];
      newRgb[i + 1] = rgbData[newIndex + 1];
      newRgb[i + 2] = rgbData[newIndex + 2];
      newRgb[i + 3] = 255; // full opacity
    }
  }
}

extern "C" int32_t *__errno() {
  static int32_t dummy_errno;
  return &dummy_errno;
}
