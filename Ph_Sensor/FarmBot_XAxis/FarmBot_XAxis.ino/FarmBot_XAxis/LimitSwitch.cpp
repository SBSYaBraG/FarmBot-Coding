/**
 * LimitSwitch.cpp
 * 
 * Implementation of limit switch detection.
 * Handles monitoring and response to limit switch triggers.
 */

#include "LimitSwitch.h"
#include "Config.h"
#include "MotorControl.h"
#include "PositionManager.h"

/**
 * Initialize the limit switch pin
 */
void initializeLimitSwitch() {
  // Set up limit switch pin with pull-up resistor
  pinMode(LIMIT_X_PIN, INPUT_PULLUP);
}

/**
 * Check if the limit switch is currently triggered
 * 
 * @param direction Current movement direction
 * @return TRUE if limit switch triggered and handled, FALSE otherwise
 */
bool checkLimitSwitch(bool direction) {
  // Check if limit switch is pressed (LOW when triggered)
  if (digitalRead(LIMIT_X_PIN) == LOW) {
    Serial.println("LIMIT SWITCH PRESSED!");
    
    // Handle differently depending on calibration mode
    if (isCalibrationMode()) {
      // In calibration mode, record this position as a limit
      if (!direction) { // CW -> minimum position
        setMinPosition(getCurrentPosition());
        Serial.print("Minimum position set: ");
        Serial.println(getMinPosition());
      } else { // CCW -> maximum position
        setMaxPosition(getCurrentPosition());
        Serial.print("Maximum position set: ");
        Serial.println(getMaxPosition());
      }
    } else {
      // Normal operation - update current position to match limit if hit
      if (!direction) { // CW -> we hit the minimum
        setCurrentPosition(getMinPosition());
        Serial.println("Hit minimum limit - position updated");
      } else { // CCW -> we hit the maximum
        setCurrentPosition(getMaxPosition());
        Serial.println("Hit maximum limit - position updated");
      }
      
      // Back off from the limit
      backOffFromLimit(!direction);
    }
    return true; // Limit was triggered
  }
  return false; // No limit triggered
}

/**
 * Back off from a triggered limit switch
 * 
 * @param direction Direction to back off (opposite of trigger direction)
 */
void backOffFromLimit(bool direction) {
  Serial.println("Backing off from limit...");
  
  // Set direction opposite to the one that triggered the limit
  setDirection(direction);
  
  // Disable acceleration for backing off
  for (int i = 0; i < BACKOFF_STEPS; i++) {
    // Move at constant speed (slowest speed for safety)
    stepMotor(MAX_STEP_DELAY, direction);
  }
  
  Serial.println("Backed off from limit");
}