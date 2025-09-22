# Kart ESC Motor Control System

A comprehensive, high-performance motor control system for kart applications using Electronic Speed Controllers (ESC). This project provides both Python and C++ implementations with advanced safety features, parallel processing, and real-time performance capabilities.

## Features

### Core Functionality
- **ESC Control**: PWM-based control for standard servo-compatible ESCs
- **Multi-Motor Support**: Control multiple motors simultaneously 
- **Safety Systems**: Emergency stop, watchdog timer, acceleration limiting
- **Real-Time Performance**: High-frequency control loops with minimal latency
- **Parallel Processing**: Multi-threaded architecture for optimal performance
- **Error Handling**: Comprehensive error catching and recovery mechanisms

### Advanced Features
- **Auto-Calibration**: Automatic ESC calibration sequence
- **Configuration Files**: Easy hardware configuration via INI files
- **Logging System**: Comprehensive logging with multiple output options
- **Interactive Control**: Command-line interface for testing and debugging
- **Hardware Monitoring**: GPIO-based emergency stop and status indication

## Hardware Requirements

### Essential Components
- **Raspberry Pi** (3B+ or 4 recommended)
- **ESC** compatible with servo PWM signals (1-2ms pulse width, 50Hz)
- **Motor** (brushless or brushed, depending on ESC)
- **Power Supply** (appropriate voltage/current for motor and ESC)

### Recommended Additions
- **Emergency Stop Button** (normally open, connects to GPIO 21)
- **Status LED** (connects to GPIO 20)
- **Proper Wiring and Connectors**
- **Heat Sinks and Cooling** (for high-power applications)

### Pin Configuration
```
GPIO 18 - Motor 1 ESC Control (PWM)
GPIO 19 - Motor 2 ESC Control (PWM) - Optional
GPIO 20 - Status LED Output
GPIO 21 - Emergency Stop Input (with pull-up resistor)
```

## Installation

### Quick Setup for Raspberry Pi
```bash
# Clone the repository
git clone <repository-url>
cd Studium_Repository

# Install dependencies and setup
make setup-rpi

# Build C++ version
make all
```

### Manual Installation

#### System Dependencies
```bash
sudo apt-get update
sudo apt-get install -y build-essential wiringpi python3-dev python3-pip
```

#### Python Dependencies
```bash
pip3 install --user RPi.GPIO
```

#### Compile C++ Version
```bash
g++ -std=c++17 -pthread -lwiringPi -lrt -o kart_control kart_control.cpp
```

## Usage

### Python Version
```bash
python3 kart.py
```

### C++ Version (Recommended for performance)
```bash
sudo ./kart_control
```

### Interactive Commands
Once running, you can use these commands:
- `f <speed>` - Set forward speed (0-100%)
- `r <speed>` - Set reverse speed (0-100%)
- `s` - Stop all motors
- `c` - Calibrate ESCs
- `e` - Emergency stop
- `reset` - Reset emergency stop
- `status` - Show system status
- `quit` - Exit program

### Example Usage
```bash
> f 25          # Set 25% forward speed
> s             # Stop motors
> c             # Calibrate ESCs
> f 50          # Set 50% forward speed
> e             # Emergency stop
> reset         # Reset emergency stop
> quit          # Exit
```

## Configuration

### Using Configuration File
Edit `kart_config.ini` to customize:
- Motor pin assignments
- PWM parameters (pulse widths, frequency)
- Safety limits (max speed, acceleration rate)
- Logging settings
- Hardware-specific options

### Example Configuration
```ini
[motor_main]
pin = 18
name = main_motor
min_pulse_width = 1.0
max_pulse_width = 2.0
neutral_pulse_width = 1.5
frequency = 50

[safety_limits]
max_acceleration_rate = 0.05
max_speed = 80.0
emergency_stop_timeout = 0.1
watchdog_timeout = 2.0
```

## Safety Features

### Built-in Safety Systems
1. **Emergency Stop**: Hardware button and software command
2. **Watchdog Timer**: Automatic stop if communication is lost
3. **Acceleration Limiting**: Prevents sudden speed changes
4. **Speed Limiting**: Configurable maximum speed limits
5. **Error Recovery**: Automatic recovery from transient errors

### Safety Guidelines
- Always test with low power/speed settings first
- Ensure proper emergency stop wiring
- Use appropriate fuses and circuit protection
- Never bypass safety systems
- Keep hands and objects clear of moving parts

## Performance Comparison

| Feature | Python Version | C++ Version |
|---------|---------------|-------------|
| Control Frequency | ~50Hz | 50Hz+ |
| Latency | ~5-10ms | <1ms |
| CPU Usage | Moderate | Low |
| Memory Usage | Higher | Lower |
| Real-time Priority | No | Yes |
| Ease of Use | Higher | Moderate |

## Troubleshooting

### Common Issues

#### "Permission Denied" Errors
```bash
# Add user to gpio group
sudo usermod -a -G gpio $USER
# Log out and log back in

# Or run with sudo (C++ version)
sudo ./kart_control
```

#### ESC Not Responding
1. Check wiring connections
2. Verify power supply
3. Run calibration sequence
4. Check ESC compatibility (servo PWM signals)

#### Emergency Stop Not Working
1. Verify GPIO 21 wiring
2. Check pull-up resistor configuration
3. Test with multimeter (should read 3.3V when not pressed)

#### Compilation Errors (C++)
```bash
# Install missing dependencies
sudo apt-get install wiringpi libwiringpi-dev

# Check compiler version
g++ --version  # Should be 7.0 or newer for C++17
```

### Debugging
- Check log files in `/tmp/kart_motor*.log`
- Use `status` command to monitor system state
- Enable debug logging in configuration
- Test with minimal hardware setup first

## Development

### Code Structure

#### Python Version (`kart.py`)
- Object-oriented design with ESCController class
- Thread-safe operation with proper locking
- Comprehensive error handling and logging
- Easy to modify and extend

#### C++ Version (`kart_control.cpp`)
- High-performance implementation
- Real-time thread scheduling
- Memory-efficient design
- Hardware-optimized GPIO control

### Contributing
1. Follow existing code style and conventions
2. Add tests for new features
3. Update documentation
4. Ensure safety features are not compromised

### Testing
```bash
# Test Python syntax
make check-python

# Test C++ compilation
make test-compile

# Run with debug logging
python3 kart.py  # Edit logging level in code
```

## License

This project is provided for educational and development purposes. Use at your own risk. Always follow proper safety procedures when working with motors and electrical systems.

## Support

For issues and questions:
1. Check this README and configuration files
2. Review log files for error messages
3. Test with minimal hardware setup
4. Ensure all dependencies are installed correctly

## Acknowledgments

- Built for practical kart motor control applications
- Inspired by RC and robotics motor control systems
- Uses industry-standard PWM servo protocols
- Designed with safety and performance in mind