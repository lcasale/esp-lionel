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

  ESP_LOGD(TAG, "Frame bytes: [0x%02X, 0x%02X, 0x%02X]", frame[0], frame[1], frame[2]);

  // Send all 3 bytes in a single write - matching Python pyserial behavior
  // The Python code uses: self._serial.write(data) with no byte-by-byte delays
  // DO NOT add delays between bytes - this breaks RS-232 frame timing!
  this->uart_->write_array(frame, 3);
  
  // Note: NOT calling flush() - the working Python code has flush() commented out
  // The UART hardware will transmit the bytes in proper sequence
  
  // Log in binary format
  char bin0[9], bin1[9], bin2[9];
  format_binary(frame[0], bin0);
  format_binary(frame[1], bin1);
  format_binary(frame[2], bin2);
  ESP_LOGD(TAG, "TMCC1 Frame Sent: %s %s %s", bin0, bin1, bin2);
}

void TMCCBus::send_tmcc1_frame_repeated(uint16_t word, uint8_t repetitions) {
  ESP_LOGD(TAG, "send_tmcc1_frame_repeated: word=0x%04X, repetitions=%u", word, repetitions);

  if (this->uart_ == nullptr) {
    ESP_LOGE(TAG, "Cannot send TMCC1 frame: UART not configured");
    return;
  }

  if (repetitions == 0) {
    repetitions = 1;
  }

  // Build the 3-byte frame: 0xFE + high byte + low byte
  uint8_t single_frame[3];
  single_frame[0] = TMCC1_HEADER;
  single_frame[1] = static_cast<uint8_t>((word >> 8) & 0xFF);
  single_frame[2] = static_cast<uint8_t>(word & 0xFF);

  // Build a buffer with the frame repeated (like Python's sound_horn does with 30 repetitions)
  // Maximum buffer size: 30 repetitions * 3 bytes = 90 bytes
  uint8_t max_reps = (repetitions > 30) ? 30 : repetitions;
  uint8_t buffer[90];  // 30 * 3 = 90 bytes max
  
  for (uint8_t i = 0; i < max_reps; i++) {
    buffer[i * 3] = single_frame[0];
    buffer[i * 3 + 1] = single_frame[1];
    buffer[i * 3 + 2] = single_frame[2];
  }
  
  // Send all frames in a single write - this is how the Python code does it
  this->uart_->write_array(buffer, max_reps * 3);
  
  ESP_LOGD(TAG, "Sent %u copies of frame [0x%02X, 0x%02X, 0x%02X] (%u bytes total)", 
           max_reps, single_frame[0], single_frame[1], single_frame[2], max_reps * 3);
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

void TMCCBus::send_test_pattern() {
  // Send test patterns to verify UART is working
  ESP_LOGW(TAG, "=== SENDING TEST PATTERN ===");
  ESP_LOGW(TAG, "This will send test bytes + horn command for engine 1 (30 repetitions)");
  
  if (this->uart_ == nullptr) {
    ESP_LOGE(TAG, "Cannot send test pattern: UART not configured");
    return;
  }
  
  // First, send alternating bit patterns (good for oscilloscope)
  uint8_t test_bytes[] = {0x55, 0xAA, 0x00, 0xFF, 0x55, 0xAA};
  ESP_LOGW(TAG, "Sending test bytes: 0x55 0xAA 0x00 0xFF 0x55 0xAA");
  this->uart_->write_array(test_bytes, sizeof(test_bytes));
  
  // Small delay before TMCC command
  vTaskDelay(pdMS_TO_TICKS(50));
  
  // Now send a valid TMCC horn command with 30 repetitions (matching Python behavior)
  ESP_LOGW(TAG, "Sending TMCC horn command for engine 1 (30 repetitions)");
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

