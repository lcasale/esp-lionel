#include "tmcc_protocol.h"

namespace tmcc {

uint16_t tmcc_make_word(TMCCObjectType type, uint8_t address, TMCCCommandClass cmd_class, uint8_t data) {
  uint16_t word = 0;

  // Mask address to 7 bits and data to 5 bits
  address &= 0x7F;
  data &= 0x1F;

  switch (type) {
    case TMCCObjectType::ENGINE:
      // Engine: 0 0 A A A A A A A C C D D D D D
      // Bits 15-14: 00
      // Bits 13-7: address (7 bits)
      // Bits 6-5: command class (2 bits)
      // Bits 4-0: data (5 bits)
      word = (static_cast<uint16_t>(address) << 7) |
             (static_cast<uint16_t>(cmd_class) << 5) |
             static_cast<uint16_t>(data);
      break;

    case TMCCObjectType::SWITCH:
      // Switch: 0 1 A A A A A A A C C D D D D D
      // Bit 14: 1
      word = (1 << 14) |
             (static_cast<uint16_t>(address) << 7) |
             (static_cast<uint16_t>(cmd_class) << 5) |
             static_cast<uint16_t>(data);
      break;

    case TMCCObjectType::ACCESSORY:
      // Accessory: 1 0 A A A A A A A C C D D D D D
      // Bit 15: 1
      word = (1 << 15) |
             (static_cast<uint16_t>(address) << 7) |
             (static_cast<uint16_t>(cmd_class) << 5) |
             static_cast<uint16_t>(data);
      break;

    case TMCCObjectType::TRAIN:
      // Train: 1 1 0 0 1 A A A A C C D D D D D
      // Bits 15-14: 11, Bits 13-11: 001
      // Address is only 4 bits for trains
      word = (0b11001 << 11) |
             (static_cast<uint16_t>(address & 0x0F) << 7) |
             (static_cast<uint16_t>(cmd_class) << 5) |
             static_cast<uint16_t>(data);
      break;

    case TMCCObjectType::ROUTE:
      // Route: 1 1 0 1 A A A A A C C D D D D D
      // Bits 15-14: 11, Bits 13-12: 01
      // Address is only 5 bits for routes
      word = (0b1101 << 12) |
             (static_cast<uint16_t>(address & 0x1F) << 7) |
             (static_cast<uint16_t>(cmd_class) << 5) |
             static_cast<uint16_t>(data);
      break;
  }

  return word;
}

uint16_t tmcc_engine_action_word(uint8_t address, TMCCEngineAction action) {
  return tmcc_make_word(TMCCObjectType::ENGINE, address, TMCCCommandClass::ACTION,
                        static_cast<uint8_t>(action));
}

uint16_t tmcc_engine_speed_word(uint8_t address, uint8_t speed) {
  // Clamp speed to 0-31
  if (speed > 31) {
    speed = 31;
  }
  return tmcc_make_word(TMCCObjectType::ENGINE, address, TMCCCommandClass::ABSOLUTE_SPEED, speed);
}

}  // namespace tmcc

