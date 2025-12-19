
# ESPHome Lionel TMCC / Legacy – Copilot Instructions

This file defines how GitHub Copilot should implement and extend the ESPHome external component that talks to a Lionel TMCC / Legacy command base over serial. It contains all protocol instructions and implementation requirements.

---

## 1. Project Context

- Platform: ESPHome external component written in C++.
- Hardware: ESP32 connected to a Lionel TMCC / Legacy base via UART.
- Serial settings: 9600 baud, 8 data bits, no parity, 1 stop bit (8N1).
- Responsibilities:
  - Build and send TMCC1 (0xFE) commands directly.
  - Optionally build Legacy/TMCC2 (0xF8/0xF9/0xFB) commands (requires LCS SER2/WiFi).
- Only TMCC1 (0xFE) commands may be injected directly into a TMCC/Legacy base without LCS hardware.

---

## 2. Component File / Class Structure

Project layout (Copilot must maintain):

components/tmcc/
- tmcc.h / tmcc.cpp → TMCCBus implementation.
- tmcc_protocol.h → Protocol constants & packing helpers.
- tmcc_engine.h / tmcc_engine.cpp → Engine/train platforms.
- tmcc_accessory.h / tmcc_accessory.cpp → Optional.

### Core Class Template
class TMCCBus : public esphome::Component {
 public:
  void set_uart(esphome::uart::UARTComponent *uart);

  // TMCC1
  void send_tmcc1_word(uint16_t word);
  void send_tmcc1_frame(uint16_t word);

  void engine_action_tmcc1(uint8_t address, uint8_t action_d);
  void engine_speed_absolute_tmcc1(uint8_t address, uint8_t step_0_31);
  void engine_speed_relative_tmcc1(uint8_t address, int8_t delta_steps);

};

---

## 3. TMCC1 Protocol – 0xFE Frames

### Frame Format
Byte 0: 0xFE  
Byte 1: high byte of 16‑bit command  
Byte 2: low byte of 16‑bit command  

### TMCC1 16‑bit Word Layout
Bit positions: 15→0  
Pattern depends on object type:

Switch:      0 1 A A A A A A A C C D D D D D  
Route:       1 1 0 1 A A A A A C C D D D D D  
Engine:      0 0 A A A A A A A C C D D D D D  
Train:       1 1 0 0 1 A A A A C C D D D D D  
Accessory:   1 0 A A A A A A A C C D D D D D  

Fields:
- A = 7‑bit TMCC ID/address  
- C = command class  
  - 00 action  
  - 01 extended  
  - 10 relative speed  
  - 11 absolute speed  
- D = 5‑bit data field

### TMCC1 Helper API Requirements

In tmcc_protocol.h:

enum class TMCCObjectType { SWITCH, ROUTE, ENGINE, TRAIN, ACCESSORY };

enum class TMCCCommandClass : uint8_t {
  ACTION = 0b00,
  EXTENDED = 0b01,
  RELATIVE_SPEED = 0b10,
  ABSOLUTE_SPEED = 0b11,
};

uint16_t tmcc_make_word(TMCCObjectType type,
                        uint8_t address_0_127,
                        TMCCCommandClass cmd_class,
                        uint8_t data_0_31);

tmcc_make_word must:
- Insert type bits.
- Place 7-bit address.
- Insert command class.
- Insert 5-bit data.

### TMCC1 Engine Actions
enum class TMCCEngineAction : uint8_t {
  FORWARD          = 0b00000,
  TOGGLE_DIRECTION = 0b00001,
  REVERSE          = 0b00011,
  BOOST            = 0b00100,
  BRAKE            = 0b00111,
  FRONT_COUPLER    = 0b00101,
  REAR_COUPLER     = 0b00110,
  // horn/bell/AUX values also from spec
};

### TMCC1 Speed

Absolute speed:
- C = 11 (ABSOLUTE_SPEED)
- D = 0–31

Relative speed:
- C = 10 (RELATIVE_SPEED)
- D maps to -5..+5 based on spec’s table.

---

## 4. ESPHome Integration Rules

- Expose high-level ESPHome abstractions:
  - engine speed (0–31)
  - add option to override the maximum engine speed exposed to home assistant (to prevente the train from going too fast and flying off the track)
  - direction
  - horn / bell
  - couplers
  - AUX functions
- Never pack raw protocol fields inside ESPHome platforms—only use helpers.
- Always follow ESPHome coding conventions and instructions from the ESPHome docs (e.g. https://esphome.io/components/external_components/ https://esphome.io/guides/yaml/)

---

## 5. Logging Requirements
- Log every TMCC1 frame sent (0xFE + 2 bytes) at DEBUG level.
  - log should be in a bit format, e.g. "TMCC1 Frame Sent: 11111110 10101010 01010101"

## 6. Copilot Constraints

- Never modify UART settings unless requested.
- Never inject TMCC2/Legacy frames unless comments note LCS requirement.
- No magic numbers—always enums/constants.
- Code must remain clean, isolated, and testable.
- C++ code should follow modern C++ best practices for example headers should have declarations only while implementations go in .cpp files.
- becuase it needs to be said again...DO NOT PUT LOGIC INSIDE .h HEADER FILES