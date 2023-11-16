# Canon_Timelapse_Switch

## Overview

This project contains embedded C code for the MSP430 microcontroller, specifically designed for camera control. The primary function of this code is to manage camera operations such as focus, capture, and LED indications through button inputs.

## Memory and Device Configuration

- Flash/FRAM usage: 870 bytes of 4KB (21.24% used)
- RAM usage: 94 bytes of 256B (36.71% used)

The device used for this project is MSP4302332.

## Device Pinouts

The pinouts for the MSP4302332 are configured as follows:

- **P1.0 to P1.7**: Various functionalities including button inputs for up, down, and shoot, and outputs for focus, capture, and LEDs.
- **P2.6 and P2.7**: Not used in this project.
- **Power Supply**: AVCC and DVCC connected to 3.3V, AVSS and DVSS connected to GND.

## Code Functionality

- **Button Controls**: Three buttons are used for different functionalities – shot, up, and down.
- **LED Indicators**: Three LEDs are used to indicate different selections time.
- **Focus and Capture Control**: The code handles the camera's focus and capture functionalities.

## Enums and Global Variables

- `Selection_t`: Enum for different time selections.
- `State_t`: Enum for the state of the camera (play or pause).
- `selectTime[]`: Array holding the time values for different selections. The time is expressed in seconds and multiplied by 4.
- `counter`, `state`, `selection`: Global variables for tracking the current state and selection.

## Main Functions

- `main()`: Sets up the device configurations and enters the main loop for handling state changes and button presses.
- `startTimer()`, `stopTimer()`: Functions to start and stop a timer.
- `activeLeds()`, `disactiveLeds()`: Functions to activate and deactivate the LEDs based on the current selection.

## Interrupt Service Routines

- **Timer A0 ISR**: Handles the timing for the focus and capture functionalities.
- **Port 1 ISR**: Handles button press events to change states and selections.

## Usage

To use this code:
1. Upload the code to an MSP4302332 microcontroller.
2. Connect the necessary buttons and LEDs as per the pinout configuration.
3. Power the microcontroller with a 3.3V supply.

## License

This project is open source and free to use. Please adhere to the appropriate usage and modification guidelines.