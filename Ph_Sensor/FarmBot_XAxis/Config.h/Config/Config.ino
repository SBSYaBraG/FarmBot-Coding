/**
 * Config.h
 * 
 * This file contains all pin definitions and system constants for the FarmBot X-Axis controller.
 * Centralizing these values makes it easier to adjust settings without searching through code.
 * 
 * Part of the FarmBot Arduino Controller system
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

// Steps per full revolution - depends on your stepper motor and driver microstepping setting
#define STEPS_PER_REV 3200

// Speed control (microseconds between steps)
#define MIN_STEP_DELAY 200   // Fastest speed (smaller delay = faster speed)
#define MAX_STEP_DELAY 1200  // Slowest speed
#define ACCEL_RATE 10        // How quickly to accelerate (microseconds to subtract per step)
#define DECEL_RATE 10        // How quickly to decelerate (microseconds to add per step)

// Safety and Recovery
#define BACKOFF_STEPS 200    // Steps to back away from limit switch when triggered

// -------------------- EEPROM STORAGE --------------------

// EEPROM addresses for storing axis limits
#define EEPROM_INITIALIZED_ADDR 0  // Address to store initialization flag
#define EEPROM_MIN_POS_ADDR 4      // Address to store minimum position (4 bytes)
#define EEPROM_MAX_POS_ADDR 8      // Address to store maximum position (4 bytes)
#define EEPROM_MAGIC_NUMBER 42     // Value to verify EEPROM was initialized

// -------------------- SERIAL COMMUNICATION --------------------

#define SERIAL_BAUD_RATE 115200    // Baud rate for communication with Raspberry Pi

#endif // CONFIG_H