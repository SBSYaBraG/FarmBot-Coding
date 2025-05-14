/**
 * PositionManager.cpp
 * 
 * Implementation of position tracking and axis limits.
 * Manages the current position, axis limits, and position-related operations.
 */

#include "PositionManager.h"
#include "Config.h"

// Position variables
long currentPosition = 0;    // Current absolute position in steps
long minPosition = 0;        // Minimum position (CW limit)
long maxPosition = 0;        // Maximum position (CCW limit)
bool limitsInitialized = false;  // Whether axis limits have been calibrated
bool calibrationMode = false;    // Whether we're currently in calibration mode

/**
 * Update the current position by adding steps
 * 
 * @param steps Number of steps to add (positive or negative)
 */
void updatePosition(long steps) {
  currentPosition += steps;
}

/**
 * Get the current absolute position
 * 
 * @return Current position in steps
 */
long getCurrentPosition() {
  return currentPosition;
}

/**
 * Set the current position to a specific value
 * Useful for zeroing or reference setting
 * 
 * @param position New position value
 */
void setCurrentPosition(long position) {
  currentPosition = position;
}

/**
 * Reset position to zero (for backward compatibility)
 */
void resetPosition() {
  currentPosition = 0;
}

/**
 * Get the minimum position (CW limit)
 * 
 * @return Minimum position in steps
 */
long getMinPosition() {
  return minPosition;
}

/**
 * Set the minimum position (CW limit)
 * 
 * @param position New minimum position
 */
void setMinPosition(long position) {
  minPosition = position;
}

/**
 * Get the maximum position (CCW limit)
 * 
 * @return Maximum position in steps
 */
long getMaxPosition() {
  return maxPosition;
}

/**
 * Set the maximum position (CCW limit)
 * 
 * @param position New maximum position
 */
void setMaxPosition(long position) {
  maxPosition = position;
}

/**
 * Check if the limits have been initialized
 * 
 * @return TRUE if limits are initialized, FALSE otherwise
 */
bool areLimitsInitialized() {
  return limitsInitialized;
}

/**
 * Set the initialized state of the limits
 * 
 * @param initialized TRUE if limits are initialized, FALSE otherwise
 */
void setLimitsInitialized(bool initialized) {
  limitsInitialized = initialized;
}

/**
 * Get the calibration mode state
 * 
 * @return TRUE if in calibration mode, FALSE otherwise
 */
bool isCalibrationMode() {
  return calibrationMode;
}

/**
 * Set the calibration mode state
 * 
 * @param mode TRUE to enable calibration mode, FALSE to disable
 */
void setCalibrationMode(bool mode) {
  calibrationMode = mode;
}

/**
 * Check if a target position is within the valid range
 * 
 * @param position Position to check
 * @return TRUE if position is within limits, FALSE otherwise
 */
bool isPositionInRange(long position) {
  // If limits aren't initialized, we can't check
  if (!limitsInitialized) return true;
  
  // Check if position is between min and max
  return (position >= minPosition && position <= maxPosition);
}

/**
 * Constrain a position to be within valid limits
 * 
 * @param position Position to constrain
 * @return The constrained position value
 */
long constrainPosition(long position) {
  // If limits aren't initialized, return as is
  if (!limitsInitialized) return position;
  
  // Constrain position to min/max range
  if (position < minPosition) return minPosition;
  if (position > maxPosition) return maxPosition;
  return position;
}

/**
 * Calculate percentage of travel (0-100%)
 * 
 * @return Percentage between min and max position
 */
int getPositionPercentage() {
  if (!limitsInitialized || (maxPosition - minPosition) == 0) {
    return 0;  // Avoid division by zero
  }
  
  // Calculate and constrain to 0-100 range
  long range = maxPosition - minPosition;
  long relPosition = currentPosition - minPosition;
  int percent = (relPosition * 100) / range;
  
  // Constrain to 0-100 range in case of slight overrun
  if (percent < 0) percent = 0;
  if (percent > 100) percent = 100;
  
  return percent;
}