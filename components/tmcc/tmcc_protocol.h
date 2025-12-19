#pragma once

#include <cstdint>

namespace tmcc {

// TMCC1 frame header byte
static constexpr uint8_t TMCC1_HEADER = 0xFE;

// Object types for TMCC1 16-bit word construction
// Bit patterns for bits 15-14 (or more for some types)
enum class TMCCObjectType : uint8_t {
  ENGINE = 0,     // 0 0 A A A A A A A C C D D D D D
  SWITCH = 1,     // 0 1 A A A A A A A C C D D D D D
  ACCESSORY = 2,  // 1 0 A A A A A A A C C D D D D D
  TRAIN = 3,      // 1 1 0 0 1 A A A A C C D D D D D
  ROUTE = 4,      // 1 1 0 1 A A A A A C C D D D D D
};

// Command class (bits 6-5 in the 16-bit word)
enum class TMCCCommandClass : uint8_t {
  ACTION = 0b00,
  EXTENDED = 0b01,
  RELATIVE_SPEED = 0b10,
  ABSOLUTE_SPEED = 0b11,
};

// Engine action codes (5-bit data field for ACTION command class)
enum class TMCCEngineAction : uint8_t {
  FORWARD = 0b00000,
  TOGGLE_DIRECTION = 0b00001,
  REVERSE = 0b00011,
  BOOST = 0b00100,
  FRONT_COUPLER = 0b00101,
  REAR_COUPLER = 0b00110,
  BRAKE = 0b00111,
  AUX1_OFF = 0b01000,
  AUX1_OPTION1 = 0b01001,
  AUX1_OPTION2 = 0b01010,
  AUX1_ON = 0b01011,
  AUX2_OFF = 0b01100,
  AUX2_OPTION1 = 0b01101,
  AUX2_OPTION2 = 0b01110,
  AUX2_ON = 0b01111,
  BLOW_HORN1 = 0b11100,
  RING_BELL = 0b11101,
  LET_OFF_SOUND = 0b11110,
  BLOW_HORN2 = 0b11111,
};

// Extended command codes (5-bit data field for EXTENDED command class)
enum class TMCCExtendedCommand : uint8_t {
  ASSIGN_TO_TRAIN = 0b00000,
  MOMENTUM_LOW = 0b01000,
  MOMENTUM_MEDIUM = 0b01001,
  MOMENTUM_HIGH = 0b01010,
  SET_ADDRESS = 0b01011,
  // Add more as needed
};

/**
 * Build a TMCC1 16-bit command word.
 *
 * Engine format: 0 0 A A A A A A A C C D D D D D
 *   - Bits 15-14: 00 (engine type)
 *   - Bits 13-7:  7-bit address (0-127)
 *   - Bits 6-5:   2-bit command class
 *   - Bits 4-0:   5-bit data
 *
 * @param type Object type (ENGINE, SWITCH, etc.)
 * @param address 7-bit address (0-127)
 * @param cmd_class Command class (ACTION, EXTENDED, RELATIVE_SPEED, ABSOLUTE_SPEED)
 * @param data 5-bit data field (0-31)
 * @return 16-bit TMCC1 command word
 */
uint16_t tmcc_make_word(TMCCObjectType type, uint8_t address, TMCCCommandClass cmd_class, uint8_t data);

/**
 * Build an engine action command word.
 *
 * @param address Engine address (0-127)
 * @param action Engine action code
 * @return 16-bit TMCC1 command word
 */
uint16_t tmcc_engine_action_word(uint8_t address, TMCCEngineAction action);

/**
 * Build an engine absolute speed command word.
 *
 * @param address Engine address (0-127)
 * @param speed Speed step (0-31)
 * @return 16-bit TMCC1 command word
 */
uint16_t tmcc_engine_speed_word(uint8_t address, uint8_t speed);

}  // namespace tmcc
