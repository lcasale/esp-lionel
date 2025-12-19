#include "tmcc.h"
#include "esphome/core/log.h"

namespace tmcc {

static const char *const TAG = "tmcc";

void TMCCBus::setup() {
  ESP_LOGCONFIG(TAG, "Setting up TMCC Bus...");
  if (this->uart_ == nullptr) {
    ESP_LOGE(TAG, "UART not configured!");
  }
}

void TMCCBus::dump_config() {
  ESP_LOGCONFIG(TAG, "TMCC Bus:");
  if (this->uart_ != nullptr) {
    ESP_LOGCONFIG(TAG, "  UART configured");
  } else {
    ESP_LOGCONFIG(TAG, "  UART not configured!");
  }
}

float TMCCBus::get_setup_priority() const {
  // Run after UART is set up
  return esphome::setup_priority::DATA;
}

void TMCCBus::set_uart(esphome::uart::UARTComponent *uart) {
  this->uart_ = uart;
}

void TMCCBus::format_binary(uint8_t byte, char *buffer) {
  for (int i = 7; i >= 0; i--) {
    buffer[7 - i] = (byte & (1 << i)) ? '1' : '0';
  }
  buffer[8] = '\0';
}

void TMCCBus::send_tmcc1_frame(uint16_t word) {
  if (this->uart_ == nullptr) {
    ESP_LOGE(TAG, "Cannot send TMCC1 frame: UART not configured");
    return;
  }

  // Build the 3-byte frame: 0xFE + high byte + low byte
  uint8_t frame[3];
  frame[0] = TMCC1_HEADER;
  frame[1] = static_cast<uint8_t>((word >> 8) & 0xFF);
  frame[2] = static_cast<uint8_t>(word & 0xFF);

  // Send the frame
  this->uart_->write_array(frame, 3);

  // Log in binary format
  char bin0[9], bin1[9], bin2[9];
  format_binary(frame[0], bin0);
  format_binary(frame[1], bin1);
  format_binary(frame[2], bin2);
  ESP_LOGD(TAG, "TMCC1 Frame Sent: %s %s %s", bin0, bin1, bin2);
}

void TMCCBus::engine_action_tmcc1(uint8_t address, TMCCEngineAction action) {
  uint16_t word = tmcc_engine_action_word(address, action);
  this->send_tmcc1_frame(word);
}

void TMCCBus::engine_speed_absolute_tmcc1(uint8_t address, uint8_t speed) {
  uint16_t word = tmcc_engine_speed_word(address, speed);
  this->send_tmcc1_frame(word);
}

}  // namespace tmcc

