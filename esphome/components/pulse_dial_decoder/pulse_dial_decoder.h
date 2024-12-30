#pragma once

#include "esphome/components/key_provider/key_provider.h"
#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"
#include <cstdlib>
#include <utility>

namespace esphome {
namespace pulse_dial_decoder {
    class PulseDialDecoder: public key_provider::KeyProvider, public Component {
      public:
        void setup() override;
        void loop() override;
        void dump_config() override;

        void set_pin(InternalGPIOPin *pin) { this->pin_ = pin; }
        void set_timeout_us(uint32_t timeout) { this->timeout_us_ = timeout; }
        void set_min_length_us(uint32_t min_length) { this->filter_us_ = min_length; }


      protected:
        static void pulse_intr(PulseDialDecoder *decoder);

        const std::string keys_ = "~1234567890";

        InternalGPIOPin* pin_;
        ISRInternalGPIOPin isr_pin_;

        uint32_t filter_us_;
        uint32_t timeout_us_;

        uint32_t last_count_;
        uint32_t total_pulses_ = 0;


        // This struct (and the two pointers) are used to pass data between the ISR and loop.
        // These two pointers are exchanged each loop.
        // Therefore you can't use data in the pointer to loop receives to set values in the pointer to loop sends.
        // As a result it's easiest if you only use these pointers to send data from the ISR to the loop.
        // (except for resetting the values)
        struct State {
          uint32_t count_ = 0;
        };
        State state_[2];
        volatile State *set_ = state_;
        volatile State *get_ = state_ + 1;

        /// Filter state for pulse mode
        struct PulseState {
          uint32_t last_intr_ = 0;
          bool last_pin_val_ = false;
        };
        PulseState pulse_state_{};
    };
}
}