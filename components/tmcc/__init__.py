import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, number, switch, button
from esphome.const import (
    CONF_ID,
    CONF_ADDRESS,
    CONF_NAME,
    ENTITY_CATEGORY_CONFIG,
)

CODEOWNERS = ["@lcasale"]
DEPENDENCIES = ["uart"]
AUTO_LOAD = ["number", "switch", "button"]

CONF_UART_ID = "uart_id"
CONF_MAX_SPEED = "max_speed"
CONF_ENGINE = "engine"
CONF_SPEED = "speed"
CONF_DIRECTION = "direction"
CONF_HORN = "horn"
CONF_BELL = "bell"
CONF_FRONT_COUPLER = "front_coupler"
CONF_REAR_COUPLER = "rear_coupler"
CONF_BOOST = "boost"
CONF_BRAKE = "brake"
CONF_TEST_BUTTON = "test_button"

# Create namespace
tmcc_ns = cg.esphome_ns.namespace("tmcc")

# C++ class references
TMCCBus = tmcc_ns.class_("TMCCBus", cg.Component)
TMCCEngine = tmcc_ns.class_("TMCCEngine", cg.Component)
TMCCEngineSpeed = tmcc_ns.class_("TMCCEngineSpeed", number.Number, cg.Component)
TMCCEngineDirection = tmcc_ns.class_("TMCCEngineDirection", switch.Switch, cg.Component)
TMCCEngineHorn = tmcc_ns.class_("TMCCEngineHorn", button.Button, cg.Component)
TMCCEngineBell = tmcc_ns.class_("TMCCEngineBell", button.Button, cg.Component)
TMCCEngineFrontCoupler = tmcc_ns.class_("TMCCEngineFrontCoupler", button.Button, cg.Component)
TMCCEngineRearCoupler = tmcc_ns.class_("TMCCEngineRearCoupler", button.Button, cg.Component)
TMCCEngineBoost = tmcc_ns.class_("TMCCEngineBoost", button.Button, cg.Component)
TMCCEngineBrake = tmcc_ns.class_("TMCCEngineBrake", button.Button, cg.Component)
TMCCTestButton = tmcc_ns.class_("TMCCTestButton", button.Button, cg.Component)


# Engine configuration schema
ENGINE_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(TMCCEngine),
        cv.Optional(CONF_ADDRESS, default=1): cv.int_range(min=0, max=127),
        cv.Optional(CONF_MAX_SPEED, default=18): cv.int_range(min=1, max=31),
        cv.Optional(CONF_SPEED): cv.maybe_simple_value(
            number.number_schema(TMCCEngineSpeed),
            key=CONF_NAME,
        ),
        cv.Optional(CONF_DIRECTION): cv.maybe_simple_value(
            switch.switch_schema(TMCCEngineDirection),
            key=CONF_NAME,
        ),
        cv.Optional(CONF_HORN): cv.maybe_simple_value(
            button.button_schema(TMCCEngineHorn),
            key=CONF_NAME,
        ),
        cv.Optional(CONF_BELL): cv.maybe_simple_value(
            button.button_schema(TMCCEngineBell),
            key=CONF_NAME,
        ),
        cv.Optional(CONF_FRONT_COUPLER): cv.maybe_simple_value(
            button.button_schema(TMCCEngineFrontCoupler),
            key=CONF_NAME,
        ),
        cv.Optional(CONF_REAR_COUPLER): cv.maybe_simple_value(
            button.button_schema(TMCCEngineRearCoupler),
            key=CONF_NAME,
        ),
        cv.Optional(CONF_BOOST): cv.maybe_simple_value(
            button.button_schema(TMCCEngineBoost),
            key=CONF_NAME,
        ),
        cv.Optional(CONF_BRAKE): cv.maybe_simple_value(
            button.button_schema(TMCCEngineBrake),
            key=CONF_NAME,
        ),
    }
)

# Main component configuration schema
CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(TMCCBus),
        cv.Required(CONF_UART_ID): cv.use_id(uart.UARTComponent),
        cv.Optional(CONF_ENGINE): ENGINE_SCHEMA,
        cv.Optional(CONF_TEST_BUTTON): cv.maybe_simple_value(
            button.button_schema(TMCCTestButton),
            key=CONF_NAME,
        ),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    # Create and register the TMCCBus instance
    bus = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(bus, config)

    # Get UART and attach to bus
    uart_component = await cg.get_variable(config[CONF_UART_ID])
    cg.add(bus.set_uart(uart_component))

    # Create test button if configured
    if CONF_TEST_BUTTON in config:
        test_button_config = config[CONF_TEST_BUTTON]
        test_button_entity = await button.new_button(test_button_config)
        await cg.register_component(test_button_entity, test_button_config)
        cg.add(test_button_entity.set_bus(bus))

    # Handle engine configuration
    if CONF_ENGINE in config:
        engine_config = config[CONF_ENGINE]

        # Create and register the TMCCEngine instance
        engine = cg.new_Pvariable(engine_config[CONF_ID])
        await cg.register_component(engine, engine_config)

        # Configure engine
        cg.add(engine.set_bus(bus))
        cg.add(engine.set_address(engine_config[CONF_ADDRESS]))
        cg.add(engine.set_max_speed(engine_config[CONF_MAX_SPEED]))

        # Create speed number entity
        if CONF_SPEED in engine_config:
            speed_config = engine_config[CONF_SPEED]
            speed_entity = await number.new_number(
                speed_config,
                min_value=0,
                max_value=engine_config[CONF_MAX_SPEED],
                step=1,
            )
            await cg.register_component(speed_entity, speed_config)
            cg.add(speed_entity.set_engine(engine))

        # Create direction switch entity
        if CONF_DIRECTION in engine_config:
            direction_config = engine_config[CONF_DIRECTION]
            direction_entity = await switch.new_switch(direction_config)
            await cg.register_component(direction_entity, direction_config)
            cg.add(direction_entity.set_engine(engine))

        # Create horn button entity
        if CONF_HORN in engine_config:
            horn_config = engine_config[CONF_HORN]
            horn_entity = await button.new_button(horn_config)
            await cg.register_component(horn_entity, horn_config)
            cg.add(horn_entity.set_engine(engine))

        # Create bell button entity
        if CONF_BELL in engine_config:
            bell_config = engine_config[CONF_BELL]
            bell_entity = await button.new_button(bell_config)
            await cg.register_component(bell_entity, bell_config)
            cg.add(bell_entity.set_engine(engine))

        # Create front coupler button entity
        if CONF_FRONT_COUPLER in engine_config:
            front_coupler_config = engine_config[CONF_FRONT_COUPLER]
            front_coupler_entity = await button.new_button(front_coupler_config)
            await cg.register_component(front_coupler_entity, front_coupler_config)
            cg.add(front_coupler_entity.set_engine(engine))

        # Create rear coupler button entity
        if CONF_REAR_COUPLER in engine_config:
            rear_coupler_config = engine_config[CONF_REAR_COUPLER]
            rear_coupler_entity = await button.new_button(rear_coupler_config)
            await cg.register_component(rear_coupler_entity, rear_coupler_config)
            cg.add(rear_coupler_entity.set_engine(engine))

        # Create boost button entity
        if CONF_BOOST in engine_config:
            boost_config = engine_config[CONF_BOOST]
            boost_entity = await button.new_button(boost_config)
            await cg.register_component(boost_entity, boost_config)
            cg.add(boost_entity.set_engine(engine))

        # Create brake button entity
        if CONF_BRAKE in engine_config:
            brake_config = engine_config[CONF_BRAKE]
            brake_entity = await button.new_button(brake_config)
            await cg.register_component(brake_entity, brake_config)
            cg.add(brake_entity.set_engine(engine))

