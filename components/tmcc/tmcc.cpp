#include "tmcc.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

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
  ESP_LOGD(TAG, "send_tmcc1_frame: word=0x%04X (%u)", word, word);

  if (this->uart_ == nullptr) {
    ESP_LOGE(TAG, "Cannot send TMCC1 frame: UART not configured");
    return;
  }

  // Build the 3-byte frame: 0xFE + high byte + low byte
  uint8_t frame[3];
  frame[0] = TMCC1_HEADER;
  frame[1] = static_cast<uint8_t>((word >> 8) & 0xFF);
  frame[2] = static_cast<uint8_t>(word & 0xFF);

  ESP_LOGI(TAG, "TX: [0x%02X, 0x%02X, 0x%02X]", frame[0], frame[1], frame[2]);

  // Send all 3 bytes in a single write
  this->uart_->write_array(frame, 3);
  
  // Flush UART to ensure transmission completes before returning
  this->uart_->flush();
}

void TMCCBus::send_tmcc1_frame_repeated(uint16_t word, uint8_t repetitions) {
  if (this->uart_ == nullptr) {
    ESP_LOGE(TAG, "Cannot send TMCC1 frame: UART not configured");
    return;
  }

  if (repetitions == 0) {
    repetitions = 1;
  }

  // Limit to 30 repetitions max (90 bytes) - used for horn duration control
  uint8_t max_reps = (repetitions > 30) ? 30 : repetitions;

  // Build the 3-byte frame: 0xFE + high byte + low byte
  uint8_t single_frame[3];
  single_frame[0] = TMCC1_HEADER;
  single_frame[1] = static_cast<uint8_t>((word >> 8) & 0xFF);
  single_frame[2] = static_cast<uint8_t>(word & 0xFF);

  ESP_LOGI(TAG, "TX repeated: [0x%02X, 0x%02X, 0x%02X] x%u", 
           single_frame[0], single_frame[1], single_frame[2], max_reps);

  // Build buffer with repeated frames
  uint8_t buffer[90];  // 30 * 3 = 90 bytes max
  for (uint8_t i = 0; i < max_reps; i++) {
    buffer[i * 3] = single_frame[0];
    buffer[i * 3 + 1] = single_frame[1];
    buffer[i * 3 + 2] = single_frame[2];
  }
  
  // Send all frames in a single write
  this->uart_->write_array(buffer, max_reps * 3);
  
  // Flush UART to ensure transmission completes
  this->uart_->flush();
}

void TMCCBus::engine_action_tmcc1(uint8_t address, TMCCEngineAction action) {
  ESP_LOGD(TAG, "engine_action_tmcc1: address=%u action=%u", address, static_cast<uint8_t>(action));
  uint16_t word = tmcc_engine_action_word(address, action);
  this->send_tmcc1_frame(word);
}

void TMCCBus::engine_action_repeated_tmcc1(uint8_t address, TMCCEngineAction action, uint8_t repetitions) {
  ESP_LOGD(TAG, "engine_action_repeated_tmcc1: address=%u action=%u repetitions=%u", 
           address, static_cast<uint8_t>(action), repetitions);
  uint16_t word = tmcc_engine_action_word(address, action);
  this->send_tmcc1_frame_repeated(word, repetitions);
}

void TMCCBus::engine_speed_absolute_tmcc1(uint8_t address, uint8_t speed) {
  ESP_LOGD(TAG, "engine_speed_absolute_tmcc1: address=%u speed=%u", address, speed);
  uint16_t word = tmcc_engine_speed_word(address, speed);
  this->send_tmcc1_frame(word);
}

void TMCCBus::system_halt() {
  // System Halt command: 0xFFFF (all bits set)
  // This matches the Python code: bytes([0xFE, 0b11111111, 0b11111111])
  ESP_LOGW(TAG, "SYSTEM HALT - Stopping all trains!");
  this->send_tmcc1_frame_repeated(0xFFFF, 10);  // Send multiple times for reliability
}

void TMCCBus::send_test_pattern() {
  ESP_LOGW(TAG, "=== SENDING TEST PATTERN ===");
  
  if (this->uart_ == nullptr) {
    ESP_LOGE(TAG, "Cannot send test pattern: UART not configured");
    return;
  }
  
  // First, send alternating bit patterns (good for oscilloscope)
  uint8_t test_bytes[] = {0x55, 0xAA, 0x00, 0xFF, 0x55, 0xAA};
  ESP_LOGW(TAG, "Sending test bytes: 0x55 0xAA 0x00 0xFF 0x55 0xAA");
  this->uart_->write_array(test_bytes, sizeof(test_bytes));
  this->uart_->flush();
  
  // Small delay before TMCC command
  vTaskDelay(pdMS_TO_TICKS(50));
  
  // Now send a valid TMCC horn command (30 reps for duration)
  ESP_LOGW(TAG, "Sending TMCC horn command for engine 1");
  this->send_tmcc1_frame_repeated(0x009C, 30);  // Horn for engine 1
  
  ESP_LOGW(TAG, "=== TEST PATTERN COMPLETE ===");
}

void TMCCBus::send_raw_bytes(const uint8_t *data, size_t len) {
  if (this->uart_ == nullptr) {
    ESP_LOGE(TAG, "Cannot send raw bytes: UART not configured");
    return;
  }
  
  ESP_LOGD(TAG, "Sending %zu raw bytes", len);
  
  // Send all bytes in a single write - no delays between bytes
  this->uart_->write_array(data, len);
  
  ESP_LOGD(TAG, "Raw bytes sent");
}

}  // namespace tmcc

