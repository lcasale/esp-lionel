#pragma once

#include "esphome/core/component.h"
#include "esphome/components/number/number.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/button/button.h"
#include "tmcc.h"

namespace tmcc {

class TMCCEngine;

/**
 * Speed control for TMCC engine (Number entity).
 * Controls absolute speed from 0 to max_speed.
 */
class TMCCEngineSpeed : public esphome::number::Number, public esphome::Component {
 public:
  void set_engine(TMCCEngine *engine);
  void setup() override;
  void dump_config() override;

 protected:
  void control(float value) override;
  TMCCEngine *engine_{nullptr};
};

/**
 * Direction control for TMCC engine (Switch entity).
 * ON = Forward, OFF = Reverse
 */
class TMCCEngineDirection : public esphome::switch_::Switch, public esphome::Component {
 public:
  void set_engine(TMCCEngine *engine);
  void setup() override;
  void dump_config() override;

 protected:
  void write_state(bool state) override;
  TMCCEngine *engine_{nullptr};
};

/**
 * Horn button for TMCC engine.
 */
class TMCCEngineHorn : public esphome::button::Button, public esphome::Component {
 public:
  void set_engine(TMCCEngine *engine);
  void dump_config() override;

 protected:
  void press_action() override;
  TMCCEngine *engine_{nullptr};
};

/**
 * Bell button for TMCC engine.
 */
class TMCCEngineBell : public esphome::button::Button, public esphome::Component {
 public:
  void set_engine(TMCCEngine *engine);
  void dump_config() override;

 protected:
  void press_action() override;
  TMCCEngine *engine_{nullptr};
};

/**
 * Front coupler button for TMCC engine.
 */
class TMCCEngineFrontCoupler : public esphome::button::Button, public esphome::Component {
 public:
  void set_engine(TMCCEngine *engine);
  void dump_config() override;

 protected:
  void press_action() override;
  TMCCEngine *engine_{nullptr};
};

/**
 * Rear coupler button for TMCC engine.
 */
class TMCCEngineRearCoupler : public esphome::button::Button, public esphome::Component {
 public:
  void set_engine(TMCCEngine *engine);
  void dump_config() override;

 protected:
  void press_action() override;
  TMCCEngine *engine_{nullptr};
};

/**
 * Boost button for TMCC engine.
 */
class TMCCEngineBoost : public esphome::button::Button, public esphome::Component {
 public:
  void set_engine(TMCCEngine *engine);
  void dump_config() override;

 protected:
  void press_action() override;
  TMCCEngine *engine_{nullptr};
};

/**
 * Brake button for TMCC engine.
 */
class TMCCEngineBrake : public esphome::button::Button, public esphome::Component {
 public:
  void set_engine(TMCCEngine *engine);
  void dump_config() override;

 protected:
  void press_action() override;
  TMCCEngine *engine_{nullptr};
};

/**
 * TMCCEngine - Main engine controller component.
 *
 * This component holds the engine configuration and provides
 * the interface for child entities to send commands.
 */
class TMCCEngine : public esphome::Component {
 public:
  TMCCEngine() = default;

  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override;

  // Configuration setters
  void set_bus(TMCCBus *bus);
  void set_address(uint8_t address);
  void set_max_speed(uint8_t max_speed);

  // Command methods (called by child entities)
  void set_speed(uint8_t speed);
  void set_direction_forward();
  void set_direction_reverse();
  void blow_horn();
  void ring_bell();
  void open_front_coupler();
  void open_rear_coupler();
  void boost();
  void brake();

  // Getters
  uint8_t get_address() const;
  uint8_t get_max_speed() const;
  uint8_t get_current_speed() const;
  bool is_forward() const;

 protected:
  TMCCBus *bus_{nullptr};
  uint8_t address_{1};
  uint8_t max_speed_{18};
  uint8_t current_speed_{0};
  bool forward_{true};
};

}  // namespace tmcc

