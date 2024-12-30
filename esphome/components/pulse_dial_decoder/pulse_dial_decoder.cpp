#include "pulse_dial_decoder.h"
#include "esphome/core/log.h"

namespace esphome {
namespace pulse_dial_decoder {
    
static const char *const TAG = "pulse_dial_decoder";

void PulseDialDecoder::setup() {
    this->pin_->setup();
    this->isr_pin_ = pin_->to_isr();

    // Set the last processed edge to now for the first timeout
    this->last_count_ = micros();

    this->pulse_state_.last_pin_val_ = this->isr_pin_.digital_read();
    this->pin_->attach_interrupt(PulseDialDecoder::pulse_intr, this, gpio::INTERRUPT_ANY_EDGE);
}

void PulseDialDecoder::dump_config() {
    ESP_LOGCONFIG(TAG, "Pulse Dial Decoder:");
    LOG_PIN("  Pin: ", this->pin_);
}

void PulseDialDecoder::loop() {

    const uint32_t now = micros();

    // Reset the count in get before we pass it back to the ISR as set
    this->get_->count_ = 0;

    // Swap out set and get to get the latest state from the ISR
    // The ISR could interrupt on any of these lines and the results would be consistent
    auto *temp = this->set_;
    this->set_ = this->get_;
    this->get_ = temp;

     // Check if we detected a pulse this loop
    if (this->get_->count_ > 0) {
        this->total_pulses_ += this->get_->count_;
        ESP_LOGVV(TAG, "Got %u new pulses at %u, last %u", this->get_->count_, now, this->last_count_);
        this->last_count_ = now;
    }

    if ((this->total_pulses_ > 0) && (now - this->last_count_ > this->timeout_us_)) {
        ESP_LOGV(TAG, "Timed out %u by %u with %u pulses", this->timeout_us_, now - this->last_count_, this->total_pulses_);
        
        if (this->total_pulses_ > 10) ESP_LOGW(TAG, "Dial decoder got %u pulses", this->total_pulses_);
        else send_key_(this->keys_[this->total_pulses_]);

        this->total_pulses_ = 0;
    }
}
void IRAM_ATTR PulseDialDecoder::pulse_intr(PulseDialDecoder *decoder) {
    const uint32_t now = micros();

    const bool pin_val = decoder->isr_pin_.digital_read();
    auto &state = decoder->pulse_state_;
    auto &set = *decoder->set_;

    const bool length = now - state.last_intr_ >= decoder->filter_us_;

    if (length && !pin_val && state.last_pin_val_) {  // Long enough low edge
        set.count_++;
    }
    
    state.last_intr_ = now;
    state.last_pin_val_ = pin_val;
}

} // namespace pulse_dial_decoder
} // namespace esphome
