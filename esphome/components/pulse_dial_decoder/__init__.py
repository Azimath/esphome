import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import key_provider
from esphome.const import CONF_ID, CONF_PIN, CONF_TIMEOUT, CONF_MIN_LENGTH

CODEOWNERS = ["@azimath"]

AUTO_LOAD = ["key_provider"]

pulse_dial_decoder_ns = cg.esphome_ns.namespace("pulse_dial_decoder")
PulseDialDecoder = pulse_dial_decoder_ns.class_(
    "PulseDialDecoder", key_provider.KeyProvider, cg.Component
)

def validate_timeout(value):
    value = cv.positive_time_period_microseconds(value)
    if value.total_minutes > 70:
        raise cv.Invalid("Maximum timeout is 70 minutes")
    return value

def validate_min_length(value):
    value = cv.positive_time_period_microseconds(value)
    if value.total_minutes > 70:
        raise cv.Invalid("Maximum minimum length is 70 minutes")
    return value

CONFIG_SCHEMA = cv.All(
    cv.COMPONENT_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(PulseDialDecoder),
            cv.Required(CONF_PIN): pins.gpio_input_pin_schema,
            cv.Optional(CONF_TIMEOUT, default="200ms"): validate_timeout,
            cv.Optional(CONF_MIN_LENGTH, default="40ms"): validate_min_length
        }
    )
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var,config)

    pin = await cg.gpio_pin_expression(config[CONF_PIN])
    cg.add(var.set_pin(pin))
    cg.add(var.set_timeout_us(config[CONF_TIMEOUT]))
    cg.add(var.set_min_length_us(config[CONF_MIN_LENGTH]))

