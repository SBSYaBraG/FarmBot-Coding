/**
 * PositionManager.cpp
 * 
 * Implementation of position tracking for the simplified FarmBot X-Axis controller.
 */

#include "PositionManager.h"
#include "Config.h"

// Position variables
long currentPosition = 0;    // Current absolute position in steps
long maxPosition = MAX_TRAVEL;  // Maximum position (default, updated if far limit is hit)

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
 * Get the maximum position limit
 * 
 * @return Maximum position in steps
 */
long getMaxPosition() {
  return maxPosition;
}

/**
 * Set the maximum position limit
 * This is updated if the far limit switch is triggered
 * 
 * @param position New maximum position
 */
void setMaxPosition(long position) {
  maxPosition = position;
}

/**
 * Calculate percentage of travel (0-100%)
 * 
 * @return Percentage between 0 and max position
 */
int getPositionPercentage() {
  if (maxPosition <= 0) {
    return 0;  // Avoid division by zero
  }
  
  // Calculate and constrain to 0-100 range
  int percent = (currentPosition * 100) / maxPosition;
  
  // Constrain to 0-100 range in case of slight overrun
  if (percent < 0) percent = 0;
  if (percent > 100) percent = 100;
  
  return percent;
}