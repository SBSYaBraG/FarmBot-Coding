/**
 * CommandProcessor.cpp
 * 
 * Implementation of serial command processing.
 * Parses and executes commands received from the Raspberry Pi.
 */

#include "CommandProcessor.h"
#include "Config.h"
#include "MotorControl.h"
#include "PositionManager.h"
#include "SystemOperations.h"
#include "EncoderInterface.h"

/**
 * Process a command string received from serial
 * Parses the command and calls the appropriate function
 * 
 * @param command The command string to process
 */
void processCommand(String command) {
  // Log the received command
  Serial.print("Command received: ");
  Serial.println(command);
  
  // Process different command types
  if (command.startsWith("X")) {
    // Relative movement command
    long steps = command.substring(1).toInt();
    processRelativeMove(steps);
  }
  else if (command.startsWith("P")) {
    // Absolute positioning command
    long targetPos = command.substring(1).toInt();
    processAbsoluteMove(targetPos);
  }
  else if (command.startsWith("C")) {
    // Calibration command
    runCalibration();
  }
  else if (command.startsWith("H")) {
    // Homing command
    runHoming();
  }
  else if (command.startsWith("R")) {
    // Report status
    reportStatus();
  }
  else if (command.startsWith("S")) {
    // Emergency stop
    emergencyStop();
  }
  else {
    // Unknown command
    Serial.println("Unknown command. Available commands:");
    Serial.println("  X#### or X-#### - Move relative steps");
    Serial.println("  P#### - Move to absolute position");
    Serial.println("  C - Run calibration");
    Serial.println("  H - Run homing sequence");
    Serial.println("  R - Report current position and limits");
    Serial.println("  S - Stop movement immediately");
  }
}

/**
 * Report the current system status
 * Prints position, limits, and other status information
 */
void reportStatus() {
  Serial.println("\n----- SYSTEM STATUS -----");
  Serial.print("Current position: ");
  Serial.println(getCurrentPosition());
  
  if (areLimitsInitialized()) {
    Serial.print("Minimum position: ");
    Serial.println(getMinPosition());
    Serial.print("Maximum position: ");
    Serial.println(getMaxPosition());
    Serial.print("Total travel range: ");
    Serial.print(getMaxPosition() - getMinPosition());
    Serial.println(" steps");
    
    int percentPosition = getPositionPercentage();
    Serial.print("Current position %: ");
    Serial.print(percentPosition);
    Serial.println("%");
  } else {
    Serial.println("NOTICE: Axis limits not calibrated");
    Serial.println("Run calibration (C) to set limits");
  }
  
  Serial.print("Encoder position: ");
  Serial.println(getEncoderPosition());
  Serial.println("------------------------\n");
}