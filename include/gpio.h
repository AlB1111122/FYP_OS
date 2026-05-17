/*https://github.com/babbleberry/rpi4-osdev/tree/master?tab=CC0-1.0-1-ov-file
 * based off*/
#pragma once
#include "mmio.h"
#include "peripheralReg.h"
class GPIO {
public:
  void pinAsAlt3(uint32_t pinNumber);
  void pinAsAlt5(uint32_t pinNumber);

private:
  MMIO mmio;
  enum {
    GPFSEL0 = reg::PERIPHERAL_BASE + 0x200000,
    GPSET0 = reg::PERIPHERAL_BASE + 0x20001C,
    GPCLR0 = reg::PERIPHERAL_BASE + 0x200028,
    GPPUPPDN0 = reg::PERIPHERAL_BASE + 0x2000E4
  };

  enum {
    GPIO_MAX_PIN = 53,
    pinFunction_OUT = 1,
    pinFunction_ALT5 = 2,
    pinFunction_ALT3 = 7
  };

  enum { Pull_None = 0, Pull_Down = 2, Pull_Up = 1 };

  uint32_t gpioCall(uint32_t pinNumber, uint32_t value, uint32_t base,
                    uint32_t fieldSz, uint32_t fieldMax);

  uint32_t pinSet(uint32_t pinNumber, uint32_t value);
  uint32_t pinClear(uint32_t pinNumber, uint32_t value);
  uint32_t pinPull(uint32_t pinNumber, uint32_t value);
  uint32_t pinFunction(uint32_t pinNumber, uint32_t value);

  void pinInitOutputWithPullNone(uint32_t pinNumber);

  void pinSetPinOutputBool(uint32_t pinNumber, uint32_t onOrOff);
};