#pragma once

#include "esphome/components/climate_ir/climate_ir.h"

namespace esphome {
namespace panasonic {

// Values for Panasonic A75C3747 IR Controllers
// Temperature
const uint8_t PANASONIC_TEMP_MIN = 16;  // Celsius
const uint8_t PANASONIC_TEMP_MAX = 30;  // Celsius

// Modes
const uint8_t PANASONIC_MODE_AUTO = 0x01;
const uint8_t PANASONIC_MODE_COOL = 0x30;
const uint8_t PANASONIC_MODE_HEAT = 0x40;
const uint8_t PANASONIC_MODE_DRY = 0x20;
const uint8_t PANASONIC_MODE_OFF = 0x00;
const uint8_t PANASONIC_MODE_ON = 0x01;

// Fan Speed
const uint8_t PANASONIC_FAN_AUTO = 0xA0;
const uint8_t PANASONIC_FAN_SILENT = 0xB0;
const uint8_t PANASONIC_FAN_1 = 0x30;
const uint8_t PANASONIC_FAN_3 = 0x50;
const uint8_t PANASONIC_FAN_5 = 0x70;

// IR Transmission
const uint32_t PANASONIC_IR_FREQUENCY = 36700;
const uint32_t PANASONIC_HEADER_MARK = 3456;
const uint32_t PANASONIC_HEADER_SPACE = 1728;
const uint32_t PANASONIC_BIT_MARK = 432;
const uint32_t PANASONIC_ONE_SPACE = 1296;
const uint32_t PANASONIC_ZERO_SPACE = 432;
const uint16_t PANASONIC_PAUSE = 10000;

// State Frame size
const uint8_t PANASONIC_STATE_FRAME_SIZE = 27;

class PanasonicClimate : public climate_ir::ClimateIR {
 public:
  PanasonicClimate()
      : climate_ir::ClimateIR(
            PANASONIC_TEMP_MIN, PANASONIC_TEMP_MAX, 1.0f, true, false,
            {climate::CLIMATE_FAN_AUTO, climate::CLIMATE_FAN_LOW, climate::CLIMATE_FAN_MEDIUM,
             climate::CLIMATE_FAN_HIGH},
            {climate::CLIMATE_SWING_OFF, climate::CLIMATE_SWING_VERTICAL}) {}

 protected:
  // Transmit via IR the state of this climate controller.
  void transmit_state() override;
  uint8_t operation_mode_();
  uint16_t fan_speed_();
  uint8_t temperature_();
    // Handle received IR Buffer
  bool on_receive(remote_base::RemoteReceiveData data) override;
  bool parse_state_frame_(const uint8_t frame[]);
};

}  // namespace panasonic
}  // namespace esphome
