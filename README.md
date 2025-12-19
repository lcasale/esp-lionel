# esp-lionel

Control a Lionel TMCC (Trainmaster Command Control) system with an ESP32 via ESPHome.

## Overview

This ESPHome external component allows you to control Lionel TMCC-equipped trains from Home Assistant. It implements the TMCC1 protocol and communicates with the TMCC command base via a serial connection.

## Features

- **Speed Control**: Set absolute speed (0-31) with configurable maximum speed limit
- **Direction Control**: Switch between forward and reverse
- **Horn**: Blow the locomotive horn
- **Bell**: Ring the locomotive bell
- **Couplers**: Open front and rear couplers
- **Boost/Brake**: Speed boost and emergency brake

## Hardware Requirements

- ESP32 development board (e.g., ESP32-DevKitC)
- Lionel TMCC Command Base
- Serial connection between ESP32 and TMCC base (TX/RX)
- Level shifter if needed (TMCC base may use different voltage levels)

### Wiring

| ESP32 Pin | TMCC Base |
|-----------|-----------|
| GPIO23 (TX) | RX |
| GPIO22 (RX) | TX |
| GND | GND |

**Note**: Verify voltage levels before connecting. The TMCC base uses RS-232 levels which may require a level shifter or MAX232 chip.

## Installation

### Using as External Component

Add the following to your ESPHome configuration:

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/lcasale/esp-lionel
      ref: main
    components: [tmcc]
```

### Local Development

For local development, clone the repository and use:

```yaml
external_components:
  - source:
      type: local
      path: /path/to/esp-lionel/components
```

## Configuration

### Basic Configuration

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/lcasale/esp-lionel
      ref: main
    components: [tmcc]

uart:
  - id: tmcc_uart
    tx_pin: GPIO23
    rx_pin: GPIO22
    baud_rate: 9600
    parity: NONE
    stop_bits: 1

tmcc:
  uart_id: tmcc_uart
  engine:
    address: 1        # TMCC engine address (0-127, default: 1)
    max_speed: 18     # Maximum speed limit (1-31, default: 18)
    speed:
      name: "Train Speed"
    direction:
      name: "Train Direction"
    horn:
      name: "Train Horn"
    bell:
      name: "Train Bell"
    front_coupler:
      name: "Train Front Coupler"
    rear_coupler:
      name: "Train Rear Coupler"
    boost:
      name: "Train Boost"
    brake:
      name: "Train Brake"
```

### Configuration Options

#### TMCC Component

| Option | Type | Required | Default | Description |
|--------|------|----------|---------|-------------|
| `uart_id` | ID | Yes | - | ID of the UART component to use |
| `engine` | Schema | No | - | Engine configuration (see below) |

#### Engine Configuration

| Option | Type | Required | Default | Description |
|--------|------|----------|---------|-------------|
| `address` | int | No | 1 | TMCC engine address (0-127) |
| `max_speed` | int | No | 18 | Maximum speed limit (1-31) |
| `speed` | Number Schema | No | - | Speed control entity |
| `direction` | Switch Schema | No | - | Direction control entity (ON=Forward) |
| `horn` | Button Schema | No | - | Horn button entity |
| `bell` | Button Schema | No | - | Bell button entity |
| `front_coupler` | Button Schema | No | - | Front coupler button entity |
| `rear_coupler` | Button Schema | No | - | Rear coupler button entity |
| `boost` | Button Schema | No | - | Boost button entity |
| `brake` | Button Schema | No | - | Brake button entity |

## Home Assistant Integration

Once the ESP32 is flashed and connected to Home Assistant via the ESPHome integration, the following entities will be available:

- **Number**: Train Speed (0 to max_speed)
- **Switch**: Train Direction (ON = Forward, OFF = Reverse)
- **Buttons**: Horn, Bell, Front Coupler, Rear Coupler, Boost, Brake

### Example Automation

```yaml
automation:
  - alias: "Start train at slow speed"
    trigger:
      - platform: state
        entity_id: input_boolean.start_train
        to: "on"
    action:
      - service: switch.turn_on
        target:
          entity_id: switch.train_direction
      - service: number.set_value
        target:
          entity_id: number.train_speed
        data:
          value: 10
```

## Protocol Details

This component implements the TMCC1 protocol:

- **Frame Format**: 0xFE (header) + 2-byte command word
- **Serial Settings**: 9600 baud, 8 data bits, no parity, 1 stop bit
- **Engine Command Word**: `0 0 A A A A A A A C C D D D D D`
  - A = 7-bit address
  - C = Command class (00=Action, 11=Absolute Speed)
  - D = 5-bit data

## Troubleshooting

### No response from trains
1. Verify UART wiring (TX/RX may need to be swapped)
2. Check voltage levels between ESP32 and TMCC base
3. Ensure TMCC base is powered and linked to track
4. Verify engine address matches configuration

### Debug logging
Enable debug logging in your ESPHome configuration:

```yaml
logger:
  level: DEBUG
```

This will show TMCC frames being sent in binary format.

## Development

### Project Structure

```
esp-lionel/
├── components/
│   └── tmcc/
│       ├── __init__.py        # ESPHome component registration
│       ├── tmcc.h             # TMCCBus class declaration
│       ├── tmcc.cpp           # TMCCBus implementation
│       ├── tmcc_protocol.h    # Protocol constants & helpers
│       ├── tmcc_protocol.cpp  # Protocol implementation
│       ├── tmcc_engine.h      # Engine platform declaration
│       └── tmcc_engine.cpp    # Engine platform implementation
├── esphome/
│   └── esp_lionel_ha.yaml     # Example configuration
└── README.md
```

### Building Locally

1. Install ESPHome: `pip install esphome`
2. Copy `esphome/secrets.yaml.example` to `esphome/secrets.yaml` and fill in your values
3. Compile: `esphome compile esphome/esp_lionel_ha.yaml`
4. Upload: `esphome upload esphome/esp_lionel_ha.yaml`

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## References

- [ESPHome External Components Documentation](https://esphome.io/components/external_components/)
- [Lionel TMCC Protocol Documentation](https://www.lionelsupport.com/Support%20Service%20Documents/71-2911-250.pdf)
