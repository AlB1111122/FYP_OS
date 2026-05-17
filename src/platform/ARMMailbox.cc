#include "ARMMailbox.h"

uint32_t ARMMailbox::writeRead(
    unsigned char channel) { // set the values of mailbox before this func
  uintptr_t addr = reinterpret_cast<uintptr_t>(&mbox);
  // 28-bit address (MSB) and 4-bit value (LSB)
  uint32_t message = static_cast<uint32_t>(addr & ~0xF) | (channel & 0xF);

  // Wait until we can write
  while (this->mmio.read(MBOX_STATUS) & MBOX_FULL)
    ;

  // Write the address of our buffer to the mailbox with the channel appended
  this->mmio.write(MBOX_WRITE, message);

  while (true) {
    // Is there a reply?
    while (this->mmio.read(MBOX_STATUS) & MBOX_EMPTY) {
      ;
    }

    // Is it a reply to our message?
    if (message == this->mmio.read(MBOX_READ)) {
      return mbox[1] == MBOX_RESPONSE; // Is it successful?
    }
  }
  return 0;
}

void ARMMailbox::write(
    unsigned char channel) { // set the values of mailbox before this func
  uintptr_t addr = reinterpret_cast<uintptr_t>(&mbox);
  // 28-bit address (MSB) and 4-bit value (LSB)
  uint32_t message = static_cast<uint32_t>(addr & ~0xF) | (channel & 0xF);
  while (this->mmio.read(MBOX_STATUS) & MBOX_FULL) {
    ;
  }

  this->mmio.write(MBOX_WRITE, message);
}

uint32_t ARMMailbox::read(unsigned char channel) {
  while (true) {
    // check reply
    while (this->mmio.read(MBOX_STATUS) & MBOX_EMPTY) {
      ;
    }
    uint32_t data = this->mmio.read(MBOX_READ);
    unsigned char readChannel = data & 0xF;
    data >>= 4;
    // Return it straight away if it's for the requested channel
    if (readChannel == channel) {
      return data;
    }
  }
}
