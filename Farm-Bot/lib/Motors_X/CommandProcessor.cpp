/**
 * CommandProcessor.cpp (Updated)
 * 
 * Implementation of serial command processing for the simplified FarmBot X-Axis controller.
 * Added encoder position to the status report.
 */

#include "CommandProcessor.h"
#include "Config.h"
#include "MotorControl.h"
#include "PositionManager.h"
#include "EncoderInterface.h"
#include "SystemOperations.h"

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
    Serial.println("  H - Run homing sequence");
    Serial.println("  R - Report current position");
    Serial.println("  S - Stop movement immediately");
  }
}

/**
 * Report the current system status
 * Prints position and other important information
 */
void reportStatus() {
  Serial.println("\n----- SYSTEM STATUS -----");
  Serial.print("Current position: ");
  Serial.println(getCurrentPosition());
  Serial.print("Maximum position: ");
  Serial.println(getMaxPosition());
  
  // Calculate percentage of travel
  int percentPosition = getPositionPercentage();
  Serial.print("Position from home: ");
  Serial.print(percentPosition);
  Serial.println("%");
  
  // Added encoder position to the status report
  Serial.print("Encoder position: ");
  Serial.println(getEncoderPosition());
  
  Serial.print("Limit switch state: ");
  Serial.println(digitalRead(LIMIT_X_PIN) == LOW ? "TRIGGERED" : "Not triggered");
  
  Serial.println("------------------------\n");
}