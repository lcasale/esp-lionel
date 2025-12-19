#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "tmcc_protocol.h"

namespace tmcc {

/**
 * TMCCBus - Main component for TMCC serial communication.
 *
 * This component handles the low-level serial communication with a
 * Lionel TMCC/Legacy command base. It sends TMCC1 0xFE frames over UART.
 */
class TMCCBus : public esphome::Component {
 public:
  TMCCBus() = default;

  // ESPHome component lifecycle
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override;

  // UART configuration
  void set_uart(esphome::uart::UARTComponent *uart);

  // TMCC1 frame sending
  void send_tmcc1_frame(uint16_t word);

  // Engine commands
  void engine_action_tmcc1(uint8_t address, TMCCEngineAction action);
  void engine_speed_absolute_tmcc1(uint8_t address, uint8_t speed);

 protected:
  esphome::uart::UARTComponent *uart_{nullptr};

  // Helper to format byte as binary string for logging
  static void format_binary(uint8_t byte, char *buffer);
};

}  // namespace tmcc

