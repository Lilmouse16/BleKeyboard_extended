# ESP32 Human-Like Typing Automation

## Overview
This project implements a human-like typing automation system using an ESP32 microcontroller. It simulates natural typing patterns, including pauses, typos, and speed variations, while following target time allocations based on video duration.

## Hardware Requirements
- ESP32-S3 DevKit (or ESP32-WROOM)
- LED indicators (Blue and Red)
- Buzzer (passive)
- USB Cable (data capable)
- Breadboard and jumper wires

## Pin Connections
### For ESP32-S3
```
Button: GPIO0 (Built-in BOOT button)
Buzzer: GPIO5
Blue LED: GPIO47 (Built-in)
Red LED: GPIO21 (External)
```

### Hardware Setup
1. **USB Connection**:
   - Connect to USB port on ESP32-S3
   - Windows will recognize it as COMx (e.g., COM7)
   - Use a data-capable USB cable

2. **LED Connections**:
   - Blue LED: GPIO47 (Built-in) or connect external
   - Red LED: GPIO21 → LED → 330Ω resistor → GND

3. **Buzzer Connection**:
   - Positive (+) → GPIO5
   - Negative (-) → GND

## Software Setup

### Development Environment
1. **Required Software**:
   - Visual Studio Code
   - PlatformIO Extension
   - ESP32 Arduino Framework

2. **PlatformIO Configuration** (`platformio.ini`):
```ini
[env:esp32-s3]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
upload_port = COM7     ; Adjust to your port
monitor_port = COM7    ; Same as upload port
monitor_speed = 115200 ; Serial monitor baud rate
```

### Project Structure
```
esp32-typing-automation/
├── include/
│   ├── aht/                  # AHT calculation system
│   ├── analysis/            # Text analysis
│   ├── timing/             # Timing management
│   ├── utils/              # Utility functions
│   └── constants.h         # Configuration constants
├── src/
│   └── [implementation files]
├── data/
│   └── text.txt            # Task descriptions
└── platformio.ini
```

## Features

### 1. Time Management (AHT System)
- Dynamic typing speed based on video duration
- Automatic time allocation for different activities
- Progress tracking and adjustment

### 2. Human-Like Behavior
- Natural typing patterns
- Random typos and corrections
- Fatigue simulation
- Variable speed and pauses

### 3. Visual Feedback
- Blue LED: Activity indicator
- Red LED: Status indicator
- Combined patterns for different states

### 4. Progress Monitoring
- Real-time progress tracking
- Speed compliance checking
- ETA calculations

## Usage

### Initial Setup
1. Clone repository
2. Open in VS Code with PlatformIO
3. Modify `constants.h` if needed
4. Connect hardware according to pin diagram

### Running the Project
1. Upload `text.txt` to SPIFFS
2. Compile and upload code
3. Open serial monitor
4. Press BOOT button to start

### LED Patterns
- Both OFF: Standby
- Alternating: Active typing
- Both ON: Connected, ready
- Red ON: Paused
- Blue ON: Waiting for connection

### Button Controls
- Single press: Start/Pause
- Double press: Skip section
- Long press: Reset current section

## Configuration

### Adjustable Parameters (`constants.h`)
```cpp
namespace Constants {
    namespace Typing {
        const int BASE_WPM = 65;        // Base typing speed
        const float MIN_SPEED_MULTIPLIER = 0.5f;
        const float MAX_SPEED_MULTIPLIER = 2.0f;
    }

    namespace HumanBehavior {
        const float TYPO_CHANCE = 0.15f;
        const float FATIGUE_FACTOR = 0.05f;
    }
}
```

## Troubleshooting

### Connection Issues
1. Check USB cable is data-capable
2. Verify correct COM port in Device Manager
3. Try pressing BOOT while connecting

### Upload Problems
1. Hold BOOT button while starting upload
2. Release after upload begins
3. Check serial monitor for errors

### LED Issues
1. Verify pin connections
2. Check LED polarity
3. Confirm current-limiting resistors

## Future Enhancements
- Multiple typing personality profiles
- Advanced fatigue modeling
- Network configuration interface
- Extended status reporting

## License
MIT License - See LICENSE file for details

## Contributors
- Initial development: [Your Name]
- Hardware testing: [Team Members]

## Version History
- v1.0.0: Initial release
- v1.1.0: ESP32-S3 support
- v1.2.0: Enhanced typing patterns
