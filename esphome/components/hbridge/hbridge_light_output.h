#pragma once

#include "esphome/core/component.h"
#include "esphome/components/output/float_output.h"
#include "esphome/components/light/light_output.h"
#include "esphome/core/log.h"

namespace esphome {
namespace hbridge {

class HBRIDGELightOutput : public PollingComponent, public light::LightOutput {
 public:
  HBRIDGELightOutput() : PollingComponent(1) {}

  void set_pina_pin(output::FloatOutput *pina_pin) { pina_pin_ = pina_pin; }
  void set_pinb_pin(output::FloatOutput *pinb_pin) { pinb_pin_ = pinb_pin; }

  light::LightTraits get_traits() override {
    auto traits = light::LightTraits();
    traits.set_supports_brightness(true);  // Dimming
    traits.set_supports_rgb(false);
    traits.set_supports_rgb_white_value(true);  // hbridge color
    traits.set_supports_color_temperature(false);
    return traits;
  }

  void setup() override {
    this->pwm_tick_count_ = 0;
  }

  void update() override {
    if (this->pwm_tick_count_ == 0) {  // First LED Direction
      this->pinb_pin_->set_level(this->duty_off_);
      this->pina_pin_->set_level(this->pina_duty_);
      this->pwm_tick_count_ = 1;
    } else {  // Second LED Direction
      this->pina_pin_->set_level(this->duty_off_);
      this->pinb_pin_->set_level(this->pinb_duty_);
      this->pwm_tick_count_ = 0;
    }
  }

  float get_setup_priority() const override { return setup_priority::HARDWARE; }

  void write_state(light::LightState *state) override {
    float bright;
    state->current_values_as_brightness(&bright);

    state->set_gamma_correct(0);
    float red, green, blue, white;
    state->current_values_as_rgbw(&red, &green, &blue, &white);

    if (white > 0.55) {
      this->pina_duty_ = (bright * (1 - white));
      this->pinb_duty_ = bright;
    } else if (white < 0.45) {
      this->pina_duty_ = bright;
      this->pinb_duty_ = (bright * (white));
    } else {
      this->pina_duty_ = bright;
      this->pinb_duty_ = bright;
    }
  }

 protected:
  output::FloatOutput *pina_pin_;
  output::FloatOutput *pinb_pin_;
  float pina_duty_ = 0;
  float pinb_duty_ = 0;
  float duty_off_ = 0;
  uint8_t pwm_tick_count_ = 0;
};

}  // namespace hbridge
}  // namespace esphome
