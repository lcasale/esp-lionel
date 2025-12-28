#include "tmcc_engine.h"
#include "esphome/core/log.h"

namespace tmcc {

static const char *const TAG = "tmcc.engine";

// ============================================================================
// TMCCEngine implementation
// ============================================================================

void TMCCEngine::setup() {
  ESP_LOGCONFIG(TAG, "Setting up TMCC Engine...");
  if (this->bus_ == nullptr) {
    ESP_LOGE(TAG, "TMCCBus not configured!");
  }
}

void TMCCEngine::dump_config() {
  ESP_LOGCONFIG(TAG, "TMCC Engine:");
  ESP_LOGCONFIG(TAG, "  Address: %u", this->address_);
  ESP_LOGCONFIG(TAG, "  Max Speed: %u", this->max_speed_);
}

float TMCCEngine::get_setup_priority() const {
  return esphome::setup_priority::DATA;
}

void TMCCEngine::set_bus(TMCCBus *bus) {
  this->bus_ = bus;
}

void TMCCEngine::set_address(uint8_t address) {
  this->address_ = address & 0x7F;  // Mask to 7 bits
}

void TMCCEngine::set_max_speed(uint8_t max_speed) {
  if (max_speed > 31) {
    max_speed = 31;
  }
  this->max_speed_ = max_speed;
}

void TMCCEngine::set_speed(uint8_t speed) {
  if (speed > this->max_speed_) {
    speed = this->max_speed_;
  }
  this->current_speed_ = speed;
  if (this->bus_ != nullptr) {
    this->bus_->engine_speed_absolute_tmcc1(this->address_, speed);
  }
}

void TMCCEngine::set_direction_forward() {
  this->forward_ = true;
  if (this->bus_ != nullptr) {
    this->bus_->engine_action_tmcc1(this->address_, TMCCEngineAction::FORWARD);
  }
}

void TMCCEngine::set_direction_reverse() {
  this->forward_ = false;
  if (this->bus_ != nullptr) {
    this->bus_->engine_action_tmcc1(this->address_, TMCCEngineAction::REVERSE);
  }
}

void TMCCEngine::blow_horn() {
  ESP_LOGI(TAG, "blow_horn: address=%u", this->address_);

  if (this->bus_ != nullptr) {
    // The Python code sends the horn command 30 times for reliable reception!
    // This is critical for the command to be recognized by the TMCC receiver.
    this->bus_->engine_action_repeated_tmcc1(this->address_, TMCCEngineAction::BLOW_HORN1, 30);
  } else {
    ESP_LOGE(TAG, "bus_ is nullptr! Cannot send horn command");
  }
}

void TMCCEngine::ring_bell() {
  ESP_LOGI(TAG, "ring_bell: address=%u", this->address_);

  if (this->bus_ != nullptr) {
    // Send bell command multiple times like horn for reliability
    this->bus_->engine_action_repeated_tmcc1(this->address_, TMCCEngineAction::RING_BELL, 30);
  }
}

void TMCCEngine::open_front_coupler() {
  if (this->bus_ != nullptr) {
    this->bus_->engine_action_tmcc1(this->address_, TMCCEngineAction::FRONT_COUPLER);
  }
}

void TMCCEngine::open_rear_coupler() {
  if (this->bus_ != nullptr) {
    this->bus_->engine_action_tmcc1(this->address_, TMCCEngineAction::REAR_COUPLER);
  }
}

void TMCCEngine::boost() {
  if (this->bus_ != nullptr) {
    this->bus_->engine_action_tmcc1(this->address_, TMCCEngineAction::BOOST);
  }
}

void TMCCEngine::brake() {
  if (this->bus_ != nullptr) {
    this->bus_->engine_action_tmcc1(this->address_, TMCCEngineAction::BRAKE);
  }
}

uint8_t TMCCEngine::get_address() const {
  return this->address_;
}

uint8_t TMCCEngine::get_max_speed() const {
  return this->max_speed_;
}

uint8_t TMCCEngine::get_current_speed() const {
  return this->current_speed_;
}

bool TMCCEngine::is_forward() const {
  return this->forward_;
}

// ============================================================================
// TMCCEngineSpeed implementation
// ============================================================================

void TMCCEngineSpeed::set_engine(TMCCEngine *engine) {
  this->engine_ = engine;
}

void TMCCEngineSpeed::setup() {
  if (this->engine_ != nullptr) {
    // Set initial traits
    this->traits.set_min_value(0);
    this->traits.set_max_value(this->engine_->get_max_speed());
    this->traits.set_step(1);
    // Publish initial state
    this->publish_state(0);
  }
}

void TMCCEngineSpeed::dump_config() {
  LOG_NUMBER("", "TMCC Engine Speed", this);
  if (this->engine_ != nullptr) {
    ESP_LOGCONFIG(TAG, "  Engine Address: %u", this->engine_->get_address());
    ESP_LOGCONFIG(TAG, "  Max Speed: %u", this->engine_->get_max_speed());
  }
}

void TMCCEngineSpeed::control(float value) {
  if (this->engine_ != nullptr) {
    uint8_t speed = static_cast<uint8_t>(value);
    this->engine_->set_speed(speed);
    this->publish_state(value);
  }
}

// ============================================================================
// TMCCEngineDirection implementation
// ============================================================================

void TMCCEngineDirection::set_engine(TMCCEngine *engine) {
  this->engine_ = engine;
}

void TMCCEngineDirection::setup() {
  // Publish initial state (forward = ON)
  this->publish_state(true);
}

void TMCCEngineDirection::dump_config() {
  LOG_SWITCH("", "TMCC Engine Direction", this);
  if (this->engine_ != nullptr) {
    ESP_LOGCONFIG(TAG, "  Engine Address: %u", this->engine_->get_address());
  }
}

void TMCCEngineDirection::write_state(bool state) {
  if (this->engine_ != nullptr) {
    if (state) {
      this->engine_->set_direction_forward();
    } else {
      this->engine_->set_direction_reverse();
    }
    this->publish_state(state);
  }
}

// ============================================================================
// TMCCEngineHorn implementation
// ============================================================================

void TMCCEngineHorn::set_engine(TMCCEngine *engine) {
  this->engine_ = engine;
}

void TMCCEngineHorn::dump_config() {
  LOG_BUTTON("", "TMCC Engine Horn", this);
  if (this->engine_ != nullptr) {
    ESP_LOGCONFIG(TAG, "  Engine Address: %u", this->engine_->get_address());
  }
}

void TMCCEngineHorn::press_action() {
  ESP_LOGI(TAG, "Horn button pressed");
  if (this->engine_ != nullptr) {
    this->engine_->blow_horn();
  } else {
    ESP_LOGE(TAG, "engine_ is nullptr! Cannot blow horn");
  }
}

// ============================================================================
// TMCCEngineBell implementation
// ============================================================================

void TMCCEngineBell::set_engine(TMCCEngine *engine) {
  this->engine_ = engine;
}

void TMCCEngineBell::dump_config() {
  LOG_BUTTON("", "TMCC Engine Bell", this);
  if (this->engine_ != nullptr) {
    ESP_LOGCONFIG(TAG, "  Engine Address: %u", this->engine_->get_address());
  }
}

void TMCCEngineBell::press_action() {
  if (this->engine_ != nullptr) {
    this->engine_->ring_bell();
  }
}

// ============================================================================
// TMCCEngineFrontCoupler implementation
// ============================================================================

void TMCCEngineFrontCoupler::set_engine(TMCCEngine *engine) {
  this->engine_ = engine;
}

void TMCCEngineFrontCoupler::dump_config() {
  LOG_BUTTON("", "TMCC Engine Front Coupler", this);
  if (this->engine_ != nullptr) {
    ESP_LOGCONFIG(TAG, "  Engine Address: %u", this->engine_->get_address());
  }
}

void TMCCEngineFrontCoupler::press_action() {
  if (this->engine_ != nullptr) {
    this->engine_->open_front_coupler();
  }
}

// ============================================================================
// TMCCEngineRearCoupler implementation
// ============================================================================

void TMCCEngineRearCoupler::set_engine(TMCCEngine *engine) {
  this->engine_ = engine;
}

void TMCCEngineRearCoupler::dump_config() {
  LOG_BUTTON("", "TMCC Engine Rear Coupler", this);
  if (this->engine_ != nullptr) {
    ESP_LOGCONFIG(TAG, "  Engine Address: %u", this->engine_->get_address());
  }
}

void TMCCEngineRearCoupler::press_action() {
  if (this->engine_ != nullptr) {
    this->engine_->open_rear_coupler();
  }
}

// ============================================================================
// TMCCEngineBoost implementation
// ============================================================================

void TMCCEngineBoost::set_engine(TMCCEngine *engine) {
  this->engine_ = engine;
}

void TMCCEngineBoost::dump_config() {
  LOG_BUTTON("", "TMCC Engine Boost", this);
  if (this->engine_ != nullptr) {
    ESP_LOGCONFIG(TAG, "  Engine Address: %u", this->engine_->get_address());
  }
}

void TMCCEngineBoost::press_action() {
  if (this->engine_ != nullptr) {
    this->engine_->boost();
  }
}

// ============================================================================
// TMCCEngineBrake implementation
// ============================================================================

void TMCCEngineBrake::set_engine(TMCCEngine *engine) {
  this->engine_ = engine;
}

void TMCCEngineBrake::dump_config() {
  LOG_BUTTON("", "TMCC Engine Brake", this);
  if (this->engine_ != nullptr) {
    ESP_LOGCONFIG(TAG, "  Engine Address: %u", this->engine_->get_address());
  }
}

void TMCCEngineBrake::press_action() {
  if (this->engine_ != nullptr) {
    this->engine_->brake();
  }
}

// ============================================================================
// TMCCTestButton implementation
// ============================================================================

void TMCCTestButton::set_bus(TMCCBus *bus) {
  this->bus_ = bus;
}

void TMCCTestButton::dump_config() {
  LOG_BUTTON("", "TMCC Test Button", this);
  ESP_LOGCONFIG(TAG, "  Use this button to send a test pattern for UART debugging");
}

void TMCCTestButton::press_action() {
  ESP_LOGW(TAG, "=== TEST BUTTON PRESSED ===");
  if (this->bus_ != nullptr) {
    this->bus_->send_test_pattern();
  } else {
    ESP_LOGE(TAG, "Cannot send test pattern: bus not configured");
  }
}

}  // namespace tmcc

