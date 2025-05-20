/**
 * Config.h (Updated)
 * 
 * Configuration constants and pin definitions for the simplified FarmBot X-Axis controller.
 * Added encoder pins.
 */

#ifndef CONFIG_H
#define CONFIG_H

// -------------------- PIN DEFINITIONS --------------------

// Motor Driver Pins
#define DIR_PIN 5        // Direction control pin
#define PUL_PIN 4        // Pulse (step) pin
#define ENA_PIN 6        // Enable pin (LOW = enabled, HIGH = disabled)

// Encoder Pins
#define ENCODER_A_PIN 2  // Encoder channel A (must be interrupt-capable pin)
#define ENCODER_B_PIN 3  // Encoder channel B (must be interrupt-capable pin)

// Limit Switch Pin
#define LIMIT_X_PIN 7    // X-axis limit switch (LOW when triggered)

// -------------------- MOTOR CONSTANTS --------------------

// Direction definitions
#define CW false         // Clockwise rotation (DIR_PIN = LOW)
#define CCW true         // Counter-clockwise rotation (DIR_PIN = HIGH)

// HOME_DIRECTION defines which direction the motor moves during homing
// CW (false) means the home switch is at the minimum position (X=0)
// CCW (true) would mean the home switch is at the maximum position
#define HOME_DIRECTION CW  // Direction to move during homing (typically CW)

// Speed control (microseconds between steps)
#define MIN_STEP_DELAY 200   // Fastest speed (smaller delay = faster speed)
#define MAX_STEP_DELAY 1000  // Slowest speed
#define ACCEL_RATE 5        // How quickly to accelerate (microseconds to subtract per step)
#define DECEL_RATE 10        // How quickly to decelerate (microseconds to add per step)

// Safety and Recovery
#define BACKOFF_STEPS 1600    // Steps to back away from limit switch when triggered
#define MAX_TRAVEL 10000000     // Default maximum travel (gets updated if far limit is hit)
#define HOMING_TIMEOUT 100000000 // Maximum steps to attempt during homing before timeout

// -------------------- SERIAL COMMUNICATION --------------------
#define SERIAL_BAUD_RATE 115200  // Baud rate for communication with Raspberry Pi

#endif // CONFIG_H