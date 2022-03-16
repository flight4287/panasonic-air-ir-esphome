import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate_ir
from esphome.const import (
    CONF_ID,
    CONF_SUPPORTED_SWING_MODES,
)
from esphome.components.climate import (
    ClimateSwingMode,
)
ALLOWED_CLIMATE_SWING_MODES = {
    "AUTO": ClimateSwingMode.CLIMATE_SWING_AUTO,
    "VERTICAL": ClimateSwingMode.CLIMATE_SWING_VERTICAL,
}
validate_swing_modes = cv.enum(ALLOWED_CLIMATE_SWING_MODES, upper=True)

AUTO_LOAD = ["climate_ir"]

panasonic_ns = cg.esphome_ns.namespace("panasonic")
PanasonicClimate = panasonic_ns.class_("PanasonicClimate", climate_ir.ClimateIR)

CONFIG_SCHEMA = climate_ir.CLIMATE_IR_WITH_RECEIVER_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(PanasonicClimate),
        cv.Optional(CONF_SUPPORTED_SWING_MODES): cv.ensure_list(
        validate_swing_modes
        ),
    }
)
async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await climate_ir.register_climate_ir(var, config)
    if CONF_SUPPORTED_SWING_MODES in config:
        cg.add(var.set_supported_swing_modes(config[CONF_SUPPORTED_SWING_MODES]))
