| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C5 | ESP32-C6 | ESP32-C61 | ESP32-H2 | ESP32-H21 | ESP32-H4 | ESP32-P4 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | --------- | -------- | --------- | -------- | -------- | -------- | -------- |

# ESP32 Motor Controller with WiFi Web Interface

A comprehensive motor control system built on ESP32 with WiFi Access Point mode and web-based interface. The system allows remote control of a motor with scheduling capabilities, real-time monitoring, and persistent storage.

## Features

- **WiFi Access Point Mode**: Creates its own WiFi network for direct connection
- **Web Interface**: Modern, responsive web UI for motor control and configuration
- **Motor Scheduling**: Add, edit, and delete motor schedules with time-based automation
- **Real-time Status**: Live motor status, system uptime, and schedule monitoring
- **Persistent Storage**: Settings and schedules saved to flash memory using NVS
- **History Tracking**: Maintains history of motor operations and schedule executions
- **Time Synchronization**: SNTP time synchronization with timezone support (GMT+5:30 Sri Lanka)
- **Device Configuration**: Customizable device name and location settings

## Hardware Requirements

- ESP32 development board (ESP32-S3 recommended)
- Motor or relay connected to GPIO 2
- Power supply suitable for your motor
- WiFi capability (built into ESP32)

## Pin Configuration

- **GPIO 2**: Motor control output (LOW = ON, HIGH = OFF)
- The pin floats when motor is OFF for safety

## WiFi Configuration

- **SSID**: Motor Controller
- **Password**: 12345679
- **IP Address**: 192.168.4.1 (default AP mode)
- **Max Connections**: 4 simultaneous clients

## Web Interface Features

### Main Dashboard
- Real-time motor status display
- System information (uptime, device name, location)
- Quick motor ON/OFF controls
- Current schedules overview
- Operation history

### Motor Control
- Manual motor control buttons
- Visual status indicators
- Safety features with floating pins when OFF

### Schedule Management
- Add new schedules with start/end times
- Enable/disable schedules
- Daily repeat functionality
- Edit and delete existing schedules
- Visual schedule list with status indicators

### Settings
- Device name configuration
- Location settings
- Time zone configuration
- Manual time setting
- Persistent storage management

## Project Structure

```
├── CMakeLists.txt
├── README.md
├── partitions.csv
├── sdkconfig
├── logo.png                   Logo image for web interface
├── main/
│   ├── CMakeLists.txt
│   ├── motor_controller.c     Main application entry point
│   ├── global.h              Global definitions and declarations
│   ├── types.h               Type definitions and constants
│   ├── motor_control.c       Motor control functions
│   ├── motor_control.h
│   ├── webserver.c           HTTP server and web interface
│   ├── webserver.h
│   ├── wifi_ap.c             WiFi Access Point configuration
│   ├── wifi_ap.h
│   ├── schedule.c            Schedule management logic
│   ├── schedule.h
│   ├── storage.c             NVS storage functions
│   ├── storage.h
│   ├── time_util.c           Time utilities and formatting
│   └── time_util.h
└── build/                    Build output directory
```

## Configuration Options

### Motor Settings
- Pin assignment (default: GPIO 2)
- Active level configuration (LOW = ON)
- Safety floating when OFF

### Time Settings
- Timezone: GMT+5:30 (Sri Lanka)
- SNTP server: pool.ntp.org
- Automatic time synchronization

### Storage Settings
- Maximum schedules: 10
- History entries: 30
- Device name max length: 32 characters

## Getting Started

### 1. Hardware Setup
1. Connect your motor or relay to GPIO 2 of the ESP32
2. Ensure proper power supply for your motor
3. Connect ESP32 to development computer

### 2. Build and Flash
```bash
# Set up ESP-IDF environment
. $HOME/esp/esp-idf/export.sh

# Navigate to project directory
cd /path/to/motor_controller

# Configure project (optional)
idf.py menuconfig

# Build the project
idf.py build

# Flash to ESP32 (replace PORT with your serial port)
idf.py -p PORT flash

# Monitor serial output
idf.py -p PORT monitor
```

### 3. Connect to WiFi
1. Power on the ESP32
2. Look for WiFi network "Motor Controller"
3. Connect with password "12345679"
4. Open web browser and go to http://192.168.4.1

### 4. Configure Device
1. Go to Settings page
2. Set device name and location
3. Configure schedules as needed
4. Settings are automatically saved to flash

## Web Interface Usage

### Accessing the Interface
- Connect to "Motor Controller" WiFi network
- Navigate to http://192.168.4.1 in web browser
- Interface is responsive and works on mobile devices

### Motor Control
- Use ON/OFF buttons for manual control
- Status is updated in real-time
- Motor blinks 3 times on startup for indication

### Schedule Management
1. Click "Add Schedule" 
2. Set start and end times
3. Choose daily repeat option
4. Enable/disable as needed
5. Schedules run automatically

### History Viewing
- View recent motor operations
- See which schedules triggered
- Monitor system activity

## API Endpoints

The system provides REST API endpoints:

- `GET /` - Main dashboard
- `GET /status` - JSON status information
- `POST /motor/on` - Turn motor ON
- `POST /motor/off` - Turn motor OFF
- `GET /schedules` - Schedule management page
- `POST /add_schedule` - Add new schedule
- `POST /delete_schedule` - Delete schedule
- `GET /settings` - Settings page
- `POST /save_settings` - Save settings
- `POST /manual_time` - Set manual time
- `GET /logo.png` - Device logo

## Troubleshooting

### Build Issues
- Ensure ESP-IDF is properly installed and sourced
- Check that all required components are available
- Clean build: `idf.py clean && idf.py build`

### WiFi Connection Issues
- Verify WiFi credentials
- Check ESP32 is in AP mode
- Reset device if network not appearing

### Motor Not Responding
- Check GPIO 2 wiring
- Verify motor power supply
- Check web interface for error messages

### Time Synchronization Issues
- Ensure internet connectivity through AP
- Check SNTP server configuration
- Manually set time if automatic sync fails

## Development

### Adding New Features
1. Add function declarations to appropriate header files
2. Implement functions in corresponding .c files
3. Update web interface if needed
4. Test thoroughly before deployment

### Code Structure
- `global.h` - Contains all global variable declarations
- `types.h` - Type definitions shared across modules
- Each module has separate .h/.c files
- Web interface uses embedded HTML/CSS/JavaScript

### Memory Management
- Uses NVS for persistent storage
- Manages flash wear leveling automatically
- Efficient memory usage for embedded system

## Technical Support

For technical support:
- Check ESP-IDF documentation
- Review ESP32 GPIO specifications
- Verify motor control circuit design
- Test with minimal hardware setup first

## License

This project is open source. Modify and distribute as needed for your applications.

