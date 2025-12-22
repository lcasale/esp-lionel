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
  // #region agent log
  ESP_LOGD(TAG, "[HYP-A] send_tmcc1_frame entry: word=0x%04X (%u)", word, word);
  // #endregion

  if (this->uart_ == nullptr) {
    ESP_LOGE(TAG, "Cannot send TMCC1 frame: UART not configured");
    return;
  }

  // Build the 3-byte frame: 0xFE + high byte + low byte
  uint8_t frame[3];
  frame[0] = TMCC1_HEADER;
  frame[1] = static_cast<uint8_t>((word >> 8) & 0xFF);
  frame[2] = static_cast<uint8_t>(word & 0xFF);

  // #region agent log
  ESP_LOGD(TAG, "[HYP-C] Frame bytes before send: [0x%02X, 0x%02X, 0x%02X] high=0x%02X low=0x%02X", 
           frame[0], frame[1], frame[2], frame[1], frame[2]);
  // #endregion

  // Send the frame byte-by-byte with small delays between bytes
  // Some RS-232 devices (like TMCC) require this for reliable reception
  // This matches common Raspberry Pi serial implementations
  // #region agent log
  ESP_LOGD(TAG, "[HYP-G] Sending frame byte-by-byte with delays");
  // #endregion
  for (int i = 0; i < 3; i++) {
    uint8_t byte_array[1] = {frame[i]};
    this->uart_->write_array(byte_array, 1);
    // Small delay between bytes to ensure proper RS-232 signal timing
    // At 9600 baud, one byte takes ~1ms, but we add a small margin
    if (i < 2) {  // Don't delay after last byte
      vTaskDelay(pdMS_TO_TICKS(2));  // 2ms delay between bytes
      // #region agent log
      ESP_LOGD(TAG, "[HYP-G] Sent byte %d: 0x%02X, waiting 2ms", i, frame[i]);
      // #endregion
    } else {
      // #region agent log
      ESP_LOGD(TAG, "[HYP-G] Sent byte %d: 0x%02X (last byte)", i, frame[i]);
      // #endregion
    }
  }

  // #region agent log
  ESP_LOGD(TAG, "[HYP-A] After write_array, calling flush");
  // #endregion

  // Flush UART to ensure data is transmitted immediately
  // This is critical for RS-232 communication where buffering can delay transmission
  this->uart_->flush();

  // #region agent log
  ESP_LOGD(TAG, "[HYP-A] After flush complete");
  // #endregion

  // Add delay to ensure transmission completes and MAX3232/RS-232 line stabilizes
  // TMCC may need time to process the frame. At 9600 baud, 3 bytes = ~3ms, but we add extra margin
  // #region agent log
  ESP_LOGD(TAG, "[HYP-F] Adding 10ms delay after transmission");
  // #endregion
  vTaskDelay(pdMS_TO_TICKS(10));  // 10ms delay

  // #region agent log
  ESP_LOGD(TAG, "[HYP-F] Delay complete");
  // #endregion

  // Log in binary format
  char bin0[9], bin1[9], bin2[9];
  format_binary(frame[0], bin0);
  format_binary(frame[1], bin1);
  format_binary(frame[2], bin2);
  ESP_LOGD(TAG, "TMCC1 Frame Sent: %s %s %s (hex: 0x%02X 0x%02X 0x%02X)", bin0, bin1, bin2, 
           frame[0], frame[1], frame[2]);
}

void TMCCBus::engine_action_tmcc1(uint8_t address, TMCCEngineAction action) {
  // #region agent log
  ESP_LOGD(TAG, "[HYP-D] engine_action_tmcc1 entry: address=%u action=%u", address, static_cast<uint8_t>(action));
  // #endregion

  uint16_t word = tmcc_engine_action_word(address, action);

  // #region agent log
  ESP_LOGD(TAG, "[HYP-D] Word constructed: 0x%04X (%u)", word, word);
  // #endregion

  this->send_tmcc1_frame(word);
}

void TMCCBus::engine_speed_absolute_tmcc1(uint8_t address, uint8_t speed) {
  // #region agent log
  ESP_LOGD(TAG, "[HYP-D] engine_speed_absolute_tmcc1 entry: address=%u speed=%u", address, speed);
  // #endregion

  uint16_t word = tmcc_engine_speed_word(address, speed);

  // #region agent log
  ESP_LOGD(TAG, "[HYP-D] Word constructed: 0x%04X (%u)", word, word);
  // #endregion

  this->send_tmcc1_frame(word);
}

}  // namespace tmcc

