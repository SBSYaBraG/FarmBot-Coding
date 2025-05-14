/**
 * FarmBot_XAxis.ino
 * 
 * Main entry point for the FarmBot X-Axis controller Arduino sketch.
 * This file initializes all modules and contains the main loop for command processing.
 * 
 * The FarmBot uses an Arduino to handle low-level motor control, limit switch detection,
 * and position tracking, while a Raspberry Pi provides high-level commands.
 */

// Include all module headers
#include "Config.h"
#include "MotorControl.h"
#include "PositionManager.h"
#include "EncoderInterface.h"
#include "LimitSwitch.h"
#include "CommandProcessor.h"
#include "StorageManager.h"
#include "SystemOperations.h"

/**
 * setup() - Arduino initialization function 
 * Runs once when the Arduino powers on or resets
 */
void setup() {
  // Initialize serial communication with Raspberry Pi
  Serial.begin(SERIAL_BAUD_RATE);
  
  // Initialize motor control pins
  initializeMotor();
  
  // Initialize encoder interface
  initializeEncoder();
  
  // Initialize limit switch
  initializeLimitSwitch();
  
  // Load saved position data from EEPROM
  if (loadAxisLimits()) {
    Serial.println("Axis limits loaded from EEPROM");
    Serial.print("Min position: ");
    Serial.print(getMinPosition());
    Serial.print(" | Max position: ");
    Serial.println(getMaxPosition());
    setLimitsInitialized(true);
  } else {
    Serial.println("No axis limits found. Run calibration first.");
  }

  // Print welcome message and available commands
  printWelcomeMessage();
}

/**
 * loop() - Arduino main loop function
 * Runs repeatedly after setup() completes
 */
void loop() {
  // Check for and process serial commands
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    
    // Process the command
    processCommand(input);
  }
}

/**
 * Print welcome message with available commands
 */
void printWelcomeMessage() {
  Serial.println("\n----- FarmBot X-Axis Controller -----");
  Serial.println("System Ready. Available commands:");
  Serial.println("  X#### or X-#### - Move relative steps (e.g., X1000)");
  Serial.println("  P#### - Move to absolute position (e.g., P5000)");
  Serial.println("  C - Run axis calibration");
  Serial.println("  H - Run homing sequence");
  Serial.println("  R - Report current position and limits");
  Serial.println("  S - Stop movement immediately");
  Serial.println("-------------------------------------");
}