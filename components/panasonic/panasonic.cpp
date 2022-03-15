#include "panasonic.h"
#include "esphome/components/remote_base/remote_base.h"

namespace esphome {
namespace panasonic {

static const char *const TAG = "panasonic.climate";

void PanasonicClimate::transmit_state() {
  uint8_t remote_state[27] = {0x02, 0x20, 0xE0, 0x04, 0x00, 0x00, 0x00, 0x06, 0x02,
                              0x20, 0xE0, 0x04, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
                              0x00, 0x06, 0x60, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00};

  remote_state[13] = this->operation_mode_();
  remote_state[14] = this->temperature_();
  uint16_t fan_speed = this->fan_speed_();
  remote_state[16] = fan_speed >> 8;

  // Calculate checksum
  for (int i = 8; i < (PANASONIC_STATE_FRAME_SIZE - 1) ; i++) {
    remote_state[26] += remote_state[i];
  }

  auto transmit = this->transmitter_->transmit();
  auto data = transmit.get_data();
  data->set_carrier_frequency(PANASONIC_IR_FREQUENCY);

  data->mark(PANASONIC_HEADER_MARK);
  data->space(PANASONIC_HEADER_SPACE);

  for (int i = 0; i < 8; i++) {
    for (uint8_t mask = 1; mask > 0; mask <<= 1) {  // iterate through bit mask
      data->mark(PANASONIC_BIT_MARK);
      bool bit = remote_state[i] & mask;
      data->space(bit ? PANASONIC_ONE_SPACE : PANASONIC_ZERO_SPACE);
    }
  }

  data->mark(PANASONIC_BIT_MARK);
  data->space(PANASONIC_PAUSE);

  data->mark(PANASONIC_HEADER_MARK);
  data->space(PANASONIC_HEADER_SPACE);

  for (int i = 8; i < 27; i++) {
    for (uint8_t mask = 1; mask > 0; mask <<= 1) {  // iterate through bit mask
      data->mark(PANASONIC_BIT_MARK);
      bool bit = remote_state[i] & mask;
      data->space(bit ? PANASONIC_ONE_SPACE : PANASONIC_ZERO_SPACE);
    }
  }
  data->mark(PANASONIC_BIT_MARK);
  data->space(0);
  transmit.perform();
}

uint8_t PanasonicClimate::operation_mode_() {
  uint8_t operating_mode = PANASONIC_MODE_ON;
  switch (this->mode) {
    case climate::CLIMATE_MODE_COOL:
      operating_mode |= PANASONIC_MODE_COOL;
      break;
    case climate::CLIMATE_MODE_DRY:
      operating_mode |= PANASONIC_MODE_DRY;
      break;
    case climate::CLIMATE_MODE_HEAT:
      operating_mode |= PANASONIC_MODE_HEAT;
      break;
    case climate::CLIMATE_MODE_AUTO:
      operating_mode |= PANASONIC_MODE_AUTO;
      break;
    case climate::CLIMATE_MODE_OFF:
    default:
      operating_mode = PANASONIC_MODE_OFF;
      break;
  }

  return operating_mode;
}

uint16_t PanasonicClimate::fan_speed_() {
  uint16_t fan_speed;
  switch (this->fan_mode.value()) {
    case climate::CLIMATE_FAN_LOW:
      fan_speed = PANASONIC_FAN_1 << 8;
      break;
    case climate::CLIMATE_FAN_MEDIUM:
      fan_speed = PANASONIC_FAN_3 << 8;
      break;
    case climate::CLIMATE_FAN_HIGH:
      fan_speed = PANASONIC_FAN_5 << 8;
      break;
    case climate::CLIMATE_FAN_AUTO:
    default:
      fan_speed = PANASONIC_FAN_AUTO << 8;
  }
  switch (this->swing_mode) {
    case climate::CLIMATE_SWING_HIGHEST:
      fan_speed |= 0x100;
      break;
    case climate::CLIMATE_SWING_HIGH:
      fan_speed |= 0x200;
      break;
    case climate::CLIMATE_SWING_MIDDLE:
      fan_speed |= 0x300;
      break;
    case climate::CLIMATE_SWING_LOW:
      fan_speed |= 0x400;
      break;
    case climate::CLIMATE_SWING_LOWEST:
      fan_speed |= 0x500;
      break;
    case climate::CLIMATE_SWING_AUTO:
      fan_speed |= 0xF00;
      break;
    default:
      break;
  }
  return fan_speed;
}

uint8_t PanasonicClimate::temperature_() {
  // Force special temperatures depending on the mode
  switch (this->mode) {
    case climate::CLIMATE_MODE_AUTO:
    case climate::CLIMATE_MODE_DRY:
      return 0xc0;
    default:
      uint8_t temperature = (uint8_t) roundf(clamp<float>(this->target_temperature, PANASONIC_TEMP_MIN, PANASONIC_TEMP_MAX));
      return temperature << 1;
  }
}

bool PanasonicClimate::parse_state_frame_(const uint8_t frame[]) {
  uint8_t checksum = 0;
  for (int i = 8; i < (PANASONIC_STATE_FRAME_SIZE - 1); i++) {
    checksum += frame[i];
  }
  if (frame[PANASONIC_STATE_FRAME_SIZE - 1] != checksum)
    return false;
  uint8_t mode = frame[13];
  if (mode & PANASONIC_MODE_ON) {
    switch (mode & 0xF0) {
      case PANASONIC_MODE_COOL:
        this->mode = climate::CLIMATE_MODE_COOL;
        break;
      case PANASONIC_MODE_DRY:
        this->mode = climate::CLIMATE_MODE_DRY;
        break;
      case PANASONIC_MODE_HEAT:
        this->mode = climate::CLIMATE_MODE_HEAT;
        break;
      case PANASONIC_MODE_AUTO:
        this->mode = climate::CLIMATE_MODE_AUTO;
        break;
    }
  } else {
    this->mode = climate::CLIMATE_MODE_OFF;
  }
  uint8_t temperature = frame[14];
  if (!(temperature & 0xC0)) {
    this->target_temperature = temperature >> 1;
  }
  
  uint8_t fan_mode = frame[16];
  uint8_t swing_mode = frame[16];
  if  (fan_mode & 0xF)
  switch (swing_mode & 0xF) {
    case PANASONIC_SWING_HIGHEST:
      this->swing_mode = climate::CLIMATE_SWING_HIGHEST;
      break;
    case PANASONIC_SWING_HIGH:
      this->swing_mode = climate::CLIMATE_SWING_HIGH;
      break;
    case PANASONIC_SWING_MIDDLE:
      this->swing_mode = climate::CLIMATE_SWING_MIDDLE;
      break;
    case PANASONIC_SWING_LOW:
      this->swing_mode = climate::CLIMATE_SWING_LOW;
      break;
    case PANASONIC_SWING_LOWEST:
      this->swing_mode = climate::CLIMATE_SWING_LOWEST;
      break;
    case PANASONIC_SWING_AUTO:
      this->swing_mode = climate::CLIMATE_SWING_AUTO;
      break;
    }
  switch (fan_mode & 0xF0) {
    case PANASONIC_FAN_1:
    case PANASONIC_FAN_2:
    case PANASONIC_FAN_SILENT:
      this->fan_mode = climate::CLIMATE_FAN_LOW;
      break;
    case PANASONIC_FAN_3:
      this->fan_mode = climate::CLIMATE_FAN_MEDIUM;
      break;
    case PANASONIC_FAN_4:
    case PANASONIC_FAN_5:
      this->fan_mode = climate::CLIMATE_FAN_HIGH;
      break;
    case PANASONIC_FAN_AUTO:
      this->fan_mode = climate::CLIMATE_FAN_AUTO;
      break;
  }
  this->publish_state();
  return true;
}

bool PanasonicClimate::on_receive(remote_base::RemoteReceiveData data) {
  uint8_t state_frame[PANASONIC_STATE_FRAME_SIZE] = {};
  if (!data.expect_item(PANASONIC_HEADER_MARK, PANASONIC_HEADER_SPACE)) {
    return false;
  }
  for (uint8_t pos = 0; pos < 8; pos++) {
    uint8_t byte = 0;
    for (int8_t bit = 0; bit < 8; bit++) {
      if (data.expect_item(PANASONIC_BIT_MARK, PANASONIC_ONE_SPACE))
        byte |= 1 << bit;
      else if (!data.expect_item(PANASONIC_BIT_MARK, PANASONIC_ZERO_SPACE)) {
        return false;
      }
    }
    state_frame[pos] = byte;
    if (pos == 0) {
      // frame header
      if (byte != 0x02)
        return false;
    } else if (pos == 1) {
      // frame header
      if (byte != 0x20)
        return false;
    } else if (pos == 2) {
      // frame header
      if (byte != 0xE0)
        return false;
    } else if (pos == 3) {
      // frame header
      if (byte != 0x04)
        return false;
    } else if (pos == 4) {
      // frame type
      if (byte != 0x00)
        return false;
    }
  }
  if (!data.expect_item(PANASONIC_BIT_MARK, PANASONIC_PAUSE)) {
    return false;
  }
  if (!data.expect_item(PANASONIC_HEADER_MARK, PANASONIC_HEADER_SPACE)) {
    return false;
  }
  for (uint8_t pos = 8; pos < PANASONIC_STATE_FRAME_SIZE; pos++) {
    uint8_t byte = 0;
    for (int8_t bit = 0; bit < 8; bit++) {
      if (data.expect_item(PANASONIC_BIT_MARK, PANASONIC_ONE_SPACE))
        byte |= 1 << bit;
      else if (!data.expect_item(PANASONIC_BIT_MARK, PANASONIC_ZERO_SPACE)) {
        return false;
      }
    }
    state_frame[pos] = byte;
  }
  return this->parse_state_frame_(state_frame);
}

}  // namespace panasonic
}  // namespace esphome
