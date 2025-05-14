/**
 * SystemOperations.cpp
 * 
 * Implementation of system operations like calibration and homing.
 * Handles special sequences for initializing and maintaining the system.
 */

#include "SystemOperations.h"
#include "Config.h"
#include "MotorControl.h"
#include "PositionManager.h"
#include "EncoderInterface.h"
#include "StorageManager.h"
#include "LimitSwitch.h"

/**
 * Run the axis calibration routine
 * Finds both CW and CCW limits and sets up axis boundaries
 */
void runCalibration() {
  Serial.println("\n===== STARTING AXIS CALIBRATION =====");
  
  // Enable calibration mode (affects limit switch handling)
  setCalibrationMode(true);
  
  // Reset position counters
  setCurrentPosition(0);
  resetEncoderPosition();
  
  // Step 1: Find CW (minimum) limit
  Serial.println("Step 1: Finding CW (minimum) limit...");
  
  // Enable motor
  enableMotor();
  
  // Move in CW direction until limit switch is triggered
  bool direction = false; // CW direction (DIR LOW)
  setDirection(direction);
  
  Serial.println("Moving CW until limit switch triggers...");
  
  // Keep stepping until limit switch triggers
  while (digitalRead(LIMIT_X_PIN) == HIGH) {
    stepMotor(MAX_STEP_DELAY, direction);
    
    // Safety check - only run for a reasonable number of steps
    if (abs(getCurrentPosition()) > 50000) {
      Serial.println("ERROR: Moved too far without finding limit");
      disableMotor();
      setCalibrationMode(false);
      return;
    }
  }
  
  Serial.println("CW limit found!");
  
  // Record the minimum position (already done in limit switch handler)
  long minPos = getCurrentPosition();
  
  // Back off from the limit
  backOffFromLimit(!direction);
  
  // Step 2: Find CCW (maximum) limit
  Serial.println("Step 2: Finding CCW (maximum) limit...");
  
  // Move in CCW direction until limit switch is triggered
  direction = true; // CCW direction (DIR HIGH)
  setDirection(direction);
  
  Serial.println("Moving CCW until limit switch triggers...");
  
  // Keep stepping until limit switch triggers
  while (digitalRead(LIMIT_X_PIN) == HIGH) {
    stepMotor(MAX_STEP_DELAY, direction);
    
    // Safety check - only run for a reasonable number of steps
    if (getCurrentPosition() > 50000) {
      Serial.println("ERROR: Moved too far without finding limit");
      disableMotor();
      setCalibrationMode(false);
      return;
    }
  }
  
  Serial.println("CCW limit found!");
  
  // Record the maximum position (already done in limit switch handler)
  long maxPos = getCurrentPosition();
  
  // Back off from the limit
  backOffFromLimit(!direction);
  
  // Step 3: Calculate the travel range
  long travelRange = maxPos - minPos;
  
  if (travelRange <= 0) {
    Serial.println("ERROR: Invalid travel range detected");
    disableMotor();
    setCalibrationMode(false);
    return;
  }
  
  // Step 4: Save the calibration to EEPROM
  saveAxisLimits();
  
  // Step 5: Mark limits as initialized
  setLimitsInitialized(true);
  
  // Step 6: Go to home position (minimum + BACKOFF_STEPS)
  Serial.println("Moving to home position...");
  
  // Calculate steps to move from current position to home
  long homePosition = getMinPosition() + BACKOFF_STEPS;
  long stepsToHome = homePosition - getCurrentPosition();
  
  if (stepsToHome != 0) {
    // Move to home position
    direction = (stepsToHome > 0);
    moveSteps(abs(stepsToHome), direction);
  }
  
  // Disable motor
  disableMotor();
  
  // Exit calibration mode
  setCalibrationMode(false);
  
  // Print results
  Serial.println("\n===== CALIBRATION COMPLETE =====");
  Serial.print("Minimum position: ");
  Serial.println(getMinPosition());
  Serial.print("Maximum position: ");
  Serial.println(getMaxPosition());
  Serial.print("Total travel range: ");
  Serial.print(travelRange);
  Serial.println(" steps");
  Serial.print("Current position: ");
  Serial.println(getCurrentPosition());
  Serial.println("================================\n");
}

/**
 * Run the homing sequence
 * Returns to a known reference position (usually minimum position)
 */
void runHoming() {
  if (!areLimitsInitialized()) {
    Serial.println("ERROR: Cannot home - axis not calibrated");
    Serial.println("Run calibration (C) first");
    return;
  }
  
  Serial.println("\n===== STARTING HOMING SEQUENCE =====");
  
  // Enable motor
  enableMotor();
  
  // Calculate home position (minimum + BACKOFF_STEPS)
  long homePosition = getMinPosition() + BACKOFF_STEPS;
  
  // Calculate steps to move from current position to home
  long stepsToHome = homePosition - getCurrentPosition();
  
  // Determine direction
  bool direction = (stepsToHome > 0);
  
  Serial.print("Moving to home position: ");
  Serial.println(homePosition);
  
  if (stepsToHome != 0) {
    // Move to home position
    moveSteps(abs(stepsToHome), direction);
  } else {
    Serial.println("Already at home position");
  }
  
  // Disable motor
  disableMotor();
  
  Serial.println("===== HOMING COMPLETE =====\n");
}