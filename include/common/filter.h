#pragma once
#include <stdint.h>

namespace com {
class Filter {
public:
  static void grayscale(uint8_t *rgbData, int32_t nPixelBits, uint8_t *newRgb);
  static void sobelEdgeDetect(uint8_t *rgbData, int32_t nPixelBits,
                              int32_t frameStride, uint8_t *newRgb);
  static void fisheyeTransform(uint8_t *rgbData, int32_t nPixelBits,
                               int32_t frameStride, uint8_t *newRgb,
                               float strength = 1.0f);
};
} // namespace com