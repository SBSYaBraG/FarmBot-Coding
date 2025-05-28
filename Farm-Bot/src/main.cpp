/**
 * FarmBot_XAxis_Simple.ino (Updated)
 * 
 * Main entry point for the simplified FarmBot X-Axis controller.
 * Added encoder initialization.
 */

// Include all module headers
#include "Config.h"
#include "MotorControl.h"
#include "PositionManager.h"
#include "EncoderInterface.h"
#include "LimitSwitch.h"
#include "CommandProcessor.h"
#include "SystemOperations.h"

/**
 * Print welcome message with available commands
 */
void printWelcomeMessage() {
  Serial.println("\n----- FarmBot X-Axis Controller (Simple) -----");
  Serial.println("System Ready. Available commands:");
  Serial.println("  X#### or X-#### - Move relative steps (e.g., X1000)");
  Serial.println("  H - Run homing sequence");
  Serial.println("  R - Report current position");
  Serial.println("  S - Stop movement immediately");
  Serial.println("-------------------------------------");
  Serial.println("IMPORTANT: Please run homing (H) after power-up to establish position reference.");
}

/**
 * setup() - Arduino initialization function 
 * Runs once when the Arduino powers on or resets
 */
void setup() {
  // Initialize serial communication
  Serial.begin(SERIAL_BAUD_RATE);
  
  // Initialize motor control pins
  initializeMotor();
  
  // Initialize encoder interface
  initializeEncoder();
  
  // Initialize limit switch
  initializeLimitSwitch();
  
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
