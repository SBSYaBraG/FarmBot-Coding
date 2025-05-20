/**
 * LimitSwitch.cpp
 * 
 * Implementation of limit switch detection for the simplified FarmBot X-Axis controller.
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
    
    // Handle differently depending on direction
    if (!direction) { // CW direction (HOME_DIRECTION) -> hitting home position
      // Set home position
      setCurrentPosition(0);
      Serial.println("Home position (0) set");
    } else { // CCW direction (opposite of HOME_DIRECTION) -> hitting far limit
      // Update maximum position
      setMaxPosition(getCurrentPosition());
      Serial.print("Maximum position updated to: ");
      Serial.println(getMaxPosition());
    }
    
    // Back off from the limit switch
    backOffFromLimit(!direction);
    return true; // Limit switch was triggered
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
  
  // Set direction to move away from the limit
  setDirection(direction);
  
  // Disable acceleration for backing off
  for (int i = 0; i < BACKOFF_STEPS; i++) {
    // Move at constant speed (slowest speed for safety)
    stepMotor(MAX_STEP_DELAY, direction);
  }
  
  Serial.println("Backed off from limit");
}