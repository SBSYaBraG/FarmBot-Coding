/**
 * MotorControl.cpp (Complete version)
 * 
 * Implementation of motor control functions with all dependencies fixed.
 */

#include "MotorControl.h"
#include "Config.h"
#include "PositionManager.h"
#include "LimitSwitch.h"

// Flag to track if emergency stop has been triggered
volatile bool emergencyStopTriggered = false;

/**
 * Initialize motor control pins
 */
void initializeMotor() {
  // Set up motor control pins
  pinMode(DIR_PIN, OUTPUT);
  pinMode(PUL_PIN, OUTPUT);
  pinMode(ENA_PIN, OUTPUT);
  
  // Start with motor disabled (save power)
  disableMotor();
}

/**
 * Enable the motor (sets the enable pin to active state)
 */
void enableMotor() {
  digitalWrite(ENA_PIN, LOW);  // LOW = enabled for most drivers
}

/**
 * Disable the motor (power saving mode, no holding torque)
 */
void disableMotor() {
  digitalWrite(ENA_PIN, HIGH); // HIGH = disabled for most drivers
}

/**
 * Set motor direction
 * 
 * @param direction TRUE for Counter-Clockwise (CCW, forward, HIGH pin state)
 *                  FALSE for Clockwise (CW, backward, LOW pin state)
 */
void setDirection(bool direction) {
  // Set direction pin
  digitalWrite(DIR_PIN, direction ? HIGH : LOW);
  
  // Small delay to ensure direction signal is stable before stepping
  delayMicroseconds(5);
}

/**
 * Move a specified number of steps in the given direction with speed control.
 * Handles acceleration and deceleration smoothly.
 * 
 * @param stepsToMove Number of steps to move
 * @param direction TRUE for Counter-Clockwise (CCW), FALSE for Clockwise (CW)
 * @return TRUE if completed successfully, FALSE if stopped by limit switch
 */
bool moveSteps(long stepsToMove, bool direction) {
  // Reset emergency stop flag
  emergencyStopTriggered = false;
  
  // Set direction pin state
  setDirection(direction);
  
  // Calculate acceleration and deceleration phases
  int stepDelay = MAX_STEP_DELAY;  // Start at slowest speed
  long totalSteps = stepsToMove;
  
  // Calculate steps for each phase (acceleration, constant speed, deceleration)
  long accelerationSteps = totalSteps / 4;  // Use 1/4 of total steps for acceleration
  long decelerationSteps = totalSteps / 4;  // Use 1/4 of total steps for deceleration
  long constantSteps = totalSteps - accelerationSteps - decelerationSteps;
  
  // Make sure we have at least some steps in each phase
  if (accelerationSteps < 10) accelerationSteps = 10;
  if (decelerationSteps < 10) decelerationSteps = 10;
  
  // Adjust if we're moving a very short distance
  if (accelerationSteps + decelerationSteps > totalSteps) {
    accelerationSteps = totalSteps / 2;
    decelerationSteps = totalSteps - accelerationSteps;
    constantSteps = 0;
  }

  // Acceleration phase
  for (long i = 0; i < accelerationSteps; i++) {
    // Check for emergency stop
    if (emergencyStopTriggered) {
      return false;
    }
    
    // Check if limit switch was triggered
    if (checkLimitSwitch(direction)) {
      return false;
    }
    
    // Generate step pulse
    stepMotor(stepDelay, direction);
    
    // Increase speed (decrease delay)
    if (stepDelay > MIN_STEP_DELAY) {
      stepDelay -= ACCEL_RATE;
      if (stepDelay < MIN_STEP_DELAY) stepDelay = MIN_STEP_DELAY;
    }
  }

  // Constant speed phase
  for (long i = 0; i < constantSteps; i++) {
    // Check for emergency stop
    if (emergencyStopTriggered) {
      return false;
    }
    
    // Check if limit switch was triggered
    if (checkLimitSwitch(direction)) {
      return false;
    }
    
    // Generate step pulse at constant speed
    stepMotor(stepDelay, direction);
  }

  // Deceleration phase
  for (long i = 0; i < decelerationSteps; i++) {
    // Check for emergency stop
    if (emergencyStopTriggered) {
      return false;
    }
    
    // Check if limit switch was triggered
    if (checkLimitSwitch(direction)) {
      return false;
    }
    
    // Generate step pulse
    stepMotor(stepDelay, direction);
    
    // Decrease speed (increase delay)
    if (stepDelay < MAX_STEP_DELAY) {
      stepDelay += DECEL_RATE;
      if (stepDelay > MAX_STEP_DELAY) stepDelay = MAX_STEP_DELAY;
    }
  }
  
  return true; // Movement completed successfully
}

/**
 * Generate a single step pulse
 * This advances the motor by one step in the current direction
 * 
 * @param delayTime Microsecond delay to wait after pulse (controls speed)
 * @param direction Direction of movement (used to update position counter)
 */
void stepMotor(int delayTime, bool direction) {
  // Generate step pulse
  digitalWrite(PUL_PIN, HIGH);
  delayMicroseconds(10);  // Pulse width (most drivers need at least 2-5µs)
  digitalWrite(PUL_PIN, LOW);
  
  // Delay to control speed (minus the pulse width)
  delayMicroseconds(delayTime - 10);

  // Update position based on direction
  updatePosition(direction ? 1 : -1);
}

/**
 * Emergency stop - immediately stop any movement
 */
void emergencyStop() {
  emergencyStopTriggered = true;
  disableMotor();
  Serial.println("EMERGENCY STOP TRIGGERED");
}

/**
 * Process a relative movement command with enhanced limit protection
 * 
 * @param steps Number of steps to move (can be positive or negative)
 */
void processRelativeMove(long steps) {
  if (steps == 0) {
    Serial.println("Zero steps requested - no movement needed");
    return;
  }
  
  Serial.print("Relative move requested: ");
  Serial.println(steps);
  
  // Calculate target position
  long currentPos = getCurrentPosition();
  long targetPosition = currentPos + steps;
  
  // Safety check to prevent moving beyond limits with margin
  if (targetPosition < BACKOFF_STEPS) {
    Serial.println("WARNING: Would move too close to home position limit!");
    Serial.print("Movement limited to safe distance (");
    Serial.print(BACKOFF_STEPS);
    Serial.println(" steps from home)");
    targetPosition = BACKOFF_STEPS;
    steps = targetPosition - currentPos;
  } 
  else if (targetPosition > (getMaxPosition() - BACKOFF_STEPS)) {
    Serial.println("WARNING: Would move too close to maximum position limit!");
    Serial.print("Movement limited to safe distance (");
    Serial.print(BACKOFF_STEPS);
    Serial.println(" steps from maximum)");
    targetPosition = getMaxPosition() - BACKOFF_STEPS;
    steps = targetPosition - currentPos;
  }
  
  // If steps changed to 0 after constraint, exit
  if (steps == 0) {
    Serial.println("Already at safe limit - no movement possible");
    return;
  }
  
  // Determine direction
  bool direction = (steps > 0);  // TRUE = CCW (HIGH), FALSE = CW (LOW)
  Serial.print("Direction: ");
  Serial.println(direction ? "CCW (forward)" : "CW (backward)");
  Serial.print("Moving from position ");
  Serial.print(currentPos);
  Serial.print(" to position ");
  Serial.println(targetPosition);
  
  // Enable motor, move, then disable
  enableMotor();
  bool success = moveSteps(abs(steps), direction);
  disableMotor();
  
  if (success) {
    Serial.print("Move complete. Current position: ");
    Serial.println(getCurrentPosition());
    Serial.print("Distance from home: ");
    Serial.print(getPositionPercentage());
    Serial.println("%");
  } else {
    Serial.println("Move interrupted by limit switch or emergency stop");
    
    // Double check current position after interruption
    Serial.print("Current position after interruption: ");
    Serial.println(getCurrentPosition());
  }
}