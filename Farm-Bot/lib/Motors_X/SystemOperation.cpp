/**
 * SystemOperations.cpp (Enhanced Homing)
 * 
 * Implementation of system operations with enhanced homing that finds both limits.
 */

#include "SystemOperations.h"
#include "Config.h"
#include "MotorControl.h"
#include "PositionManager.h"
#include "EncoderInterface.h"
#include "LimitSwitch.h"

// Flag to track if emergency stop has been triggered (shared with motor control)
extern volatile bool emergencyStopTriggered;

/**
 * Run the enhanced homing sequence
 * Finds BOTH home and far limits to establish the complete axis dimensions
 */
void runHoming() {
  Serial.println("\n===== STARTING ENHANCED HOMING SEQUENCE =====");
  
  // Enable motor
  enableMotor();
  
  // Reset emergency stop flag
  emergencyStopTriggered = false;
  
  // PART 1: Find home position (minimum limit)
  Serial.println("STEP 1: Finding home position (minimum limit)...");
  
  // Move in CW direction (HOME_DIRECTION) until limit switch is triggered
  bool direction = HOME_DIRECTION; // Typically CW (DIR LOW)
  setDirection(direction);
  
  // Counter to prevent infinite loops
  long safety_counter = 0;
  
  // Keep stepping until limit switch triggers
  while (digitalRead(LIMIT_X_PIN) == HIGH) {
    // Check for emergency stop
    if (emergencyStopTriggered) {
      Serial.println("Homing aborted by emergency stop");
      disableMotor();
      return;
    }
    
    // Step at a constant speed for reliability
    digitalWrite(PUL_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(PUL_PIN, LOW);
    delayMicroseconds(MAX_STEP_DELAY - 10);
    
    // Update position counter
    updatePosition(direction == CCW ? 1 : -1);
    
    // Safety check - only run for a reasonable number of steps
    safety_counter++;
    if (safety_counter > HOMING_TIMEOUT) {
      Serial.println("ERROR: Moved too far without finding home limit");
      Serial.println("Check limit switch wiring or adjust HOMING_TIMEOUT");
      disableMotor();
      return;
    }
  }
  
  Serial.println("Home limit switch found!");
  
  // Set the current position to 0
  setCurrentPosition(0);
  
  // Reset encoder position as well
  resetEncoderPosition();
  
  // Back off from the limit
  backOffFromLimit(!direction);
  
  // PART 2: Find far position (maximum limit)
  Serial.println("\nSTEP 2: Finding far position (maximum limit)...");
  
  // Move in opposite direction (typically CCW) until limit switch is triggered
  direction = !HOME_DIRECTION; // Typically CCW (DIR HIGH)
  setDirection(direction);
  
  // Reset safety counter
  safety_counter = 0;
  
  // Keep stepping until limit switch triggers
  while (digitalRead(LIMIT_X_PIN) == HIGH) {
    // Check for emergency stop
    if (emergencyStopTriggered) {
      Serial.println("Homing aborted by emergency stop");
      disableMotor();
      return;
    }
    
    // Step at a constant speed for reliability
    digitalWrite(PUL_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(PUL_PIN, LOW);
    delayMicroseconds(MAX_STEP_DELAY - 10);
    
    // Update position counter
    updatePosition(direction == CCW ? 1 : -1);
    
    // Safety check - only run for a reasonable number of steps
    safety_counter++;
    if (safety_counter > HOMING_TIMEOUT) {
      Serial.println("ERROR: Moved too far without finding far limit");
      Serial.println("Check limit switch wiring or adjust HOMING_TIMEOUT");
      disableMotor();
      return;
    }
  }
  
  Serial.println("Far limit switch found!");
  
  // Record the maximum position
  long maxPos = getCurrentPosition();
  setMaxPosition(maxPos);
  
  Serial.print("Maximum travel distance: ");
  Serial.print(maxPos);
  Serial.println(" steps");
  
  // Back off from the far limit
  backOffFromLimit(!direction);
  
  // PART 3: Move to center position
  Serial.println("\nSTEP 3: Moving to center position...");
  
  // Calculate center position
  long centerPos = maxPos / 2;
  
  // Move to center
  long stepsToCenter = centerPos - getCurrentPosition();
  bool centerDirection = (stepsToCenter > 0);
  
  if (stepsToCenter != 0) {
    setDirection(centerDirection);
    moveSteps(abs(stepsToCenter), centerDirection);
  }
  
  // Disable motor
  disableMotor();
  
  Serial.println("\n===== HOMING COMPLETE =====");
  Serial.println("Position counter has been zeroed at home position");
  Serial.println("Maximum travel distance has been measured");
  Serial.print("Total axis travel: ");
  Serial.print(getMaxPosition());
  Serial.println(" steps");
  Serial.println("Axis is now positioned at center");
}