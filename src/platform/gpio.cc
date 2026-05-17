#include "gpio.h"

uint32_t GPIO::gpioCall(uint32_t pinNumber, uint32_t value, uint32_t base,
                        uint32_t fieldSz, uint32_t fieldMax) {
  // mask the other bits of the register
  uint32_t fieldMask = (1 << fieldSz) - 1;

  if (pinNumber > fieldMax || value > fieldMask) {
    return 0;
  }

  // 32 bit registers on this hardware
  // gpio reges controll multipule pins calc n pins conrolled by this reg
  uint32_t nFields = 32 / fieldSz;
  // calc target pin register address
  uint32_t reg = base + ((pinNumber / nFields) * 4);
  // shift to get to the right bits of the register
  uint32_t shift = (pinNumber % nFields) * fieldSz;

  // save old state
  uint32_t curVal = this->mmio.read(reg);
  // clear target pins bits, leave the others
  curVal &= ~(fieldMask << shift);
  // instert the new value to for target bits
  curVal |= value << shift;
  this->mmio.write(reg, curVal);

  return 1;
}

uint32_t GPIO::pinSet(uint32_t pinNumber, uint32_t value) {
  return gpioCall(pinNumber, value, GPSET0, 1, GPIO_MAX_PIN);
}
uint32_t GPIO::pinClear(uint32_t pinNumber, uint32_t value) {
  return gpioCall(pinNumber, value, GPCLR0, 1, GPIO_MAX_PIN);
}
uint32_t GPIO::pinPull(uint32_t pinNumber, uint32_t value) {
  return gpioCall(pinNumber, value, GPPUPPDN0, 2, GPIO_MAX_PIN);
}
uint32_t GPIO::pinFunction(uint32_t pinNumber, uint32_t value) {
  return gpioCall(pinNumber, value, GPFSEL0, 3, GPIO_MAX_PIN);
}

void GPIO::pinAsAlt3(uint32_t pinNumber) {
  pinPull(pinNumber, Pull_None);
  pinFunction(pinNumber, pinFunction_ALT3);
}

void GPIO::pinAsAlt5(uint32_t pinNumber) {
  pinPull(pinNumber, Pull_None);
  pinFunction(pinNumber, pinFunction_ALT5);
}

void GPIO::pinInitOutputWithPullNone(uint32_t pinNumber) {
  pinPull(pinNumber, Pull_None);
  pinFunction(pinNumber, pinFunction_OUT);
}

void GPIO::pinSetPinOutputBool(uint32_t pinNumber, uint32_t onOrOff) {
  if (onOrOff) {
    pinSet(pinNumber, 1);
  } else {
    pinClear(pinNumber, 1);
  }
}