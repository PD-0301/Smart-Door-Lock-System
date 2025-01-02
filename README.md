# Smart Door Lock System

## Overview
The Smart Door Lock System leverages **RFID technology** and a **keypad interface** for enhanced access control. Designed for security and efficiency, the system provides multiple ways to authenticate access while maintaining a robust database for user management. It also includes additional features like presence detection, door open alarms, and user ID management, ensuring a modern and reliable solution for smart access.

## Features
- **RFID Fast Access**: Secure and quick authentication using RFID tags.
- **Keypad Backup**: Alternative numeric code authentication in case RFID fails.
- **Presence Detection**: Power-saving mode with automatic activation upon detecting a user.
- **Intruder Alarm**: Alerts triggered for unauthorized attempts.
- **User Management**: 
  - Automatic user ID allocation.
  - Block, unblock, or delete user IDs.
- **Door Open Alarm**: Alerts if the door remains open for an extended period.
- **Auto Unlock Mechanism**: Seamless access for authorized users.

## Working Model 
[Working Model](model.png)

## Circuit Diagram
[Circuit diagram](circuitdiagram.png)

## Flow Chart
[Flow Chart](flowchart.png)
## Components
| Component              | Quantity | Description                              |
|------------------------|----------|------------------------------------------|
| Teensy 3.5             | 1        | Microcontroller with SD card support.    |
| Solenoid 12V           | 1        | Enables door lock/unlock functionality.  |
| TIP120 Transistor       | 1        | Controls solenoid operation.             |
| Keypad (16-key)        | 1        | Numeric input for backup authentication. |
| RFID Module            | 1        | Reads RFID tags for user authentication. |
| Micro-SD Card          | 1        | Stores user credentials in a database.   |
| Power Supply (12V)     | 1        | Provides stable power to the system.     |
| 12-5V Buck Converter   | 1        | Steps down power for low-voltage components. |
| IR Sensor              | 1        | Detects door state (open/closed).        |
| Parallax LCD Display   | 1        | Displays system messages and prompts.    |

## System Workflow
1. **User Detection**:
   - An IR sensor detects if a user is near the door.
2. **Authentication**:
   - RFID tag or numeric code input via keypad.
   - Credentials verified against the database stored in the SD card.
3. **Access Grant**:
   - If credentials match, the solenoid unlocks the door for a predefined period.
4. **Door Monitoring**:
   - IR sensor checks if the door is open.
   - Triggers an alarm if the door remains open for too long.
5. **User Management**:
   - Master user can add, block, unblock, or delete user IDs.

## Advantages
- **Multiple Authentication Methods**: Combines RFID and keypad for secure access.
- **Scalable User Management**: Supports a virtually infinite number of users with SD card storage.
- **Energy Efficiency**: Presence detection minimizes unnecessary power consumption.

## Disadvantages
- Loss of the master access code locks the entire system.
- Lack of advanced connectivity features like WiFi or Bluetooth in the current iteration.
- Hardcoded master ID presents a potential security vulnerability.

## Cost Analysis
| Component              | Cost (USD) | Quantity | Total Cost (USD) |
|------------------------|------------|----------|------------------|
| Teensy 3.5             | $32.50     | 1        | $32.50           |
| Solenoid 12V           | $8.20      | 1        | $8.20            |
| TIP120 Transistor       | $6.70      | 1        | $6.70            |
| Keypad (16-key)        | $9.80      | 1        | $9.80            |
| RFID Module            | $6.20      | 1        | $6.20            |
| Micro-SD Card          | $9.00      | 1        | $9.00            |
| Power Supply (12V)     | $8.70      | 1        | $8.70            |
| 12-5V Buck Converter   | $7.40      | 1        | $7.40            |
| IR Sensor              | $4.50      | 1        | $4.50            |
| Miscellaneous          | $15.00     | 1        | $15.00           |
| **Total**              |            |          | **$188.40**      |

## Future Improvements
- **Surveillance Camera**:
  - Integrate ESP32CAM for facial recognition and surveillance.
- **Master App**:
  - Log entries and send warnings to the master user via the internet.
- **Remote Access**:
  - Enable WiFi calling for guest access.
- **Mobile App Integration**:
  - Monitor users and edit features via a smartphone.
- **Auto Door Closing**:
  - Automatically close the door after a period of inactivity.

## Getting Started
### Prerequisites
- Arduino IDE installed on your computer.
- Required hardware components as listed.

### Setup Instructions
1. Connect the components according to the circuit diagram provided in the repository.
2. Flash the code to the Teensy 3.5 microcontroller using Arduino IDE.
3. Insert the SD card with a pre-configured user database.
4. Power the system and begin testing the functionality.

## Code
The full source code is available in the `src/` folder of this repository.

