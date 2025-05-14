/**
 * PositionManager.h
 * 
 * Header file for position tracking and axis limits.
 * Manages the current position, axis limits, and position-related operations.
 */

#ifndef POSITION_MANAGER_H
#define POSITION_MANAGER_H

#include <Arduino.h>

/**
 * Update the current position by adding steps
 * 
 * @param steps Number of steps to add (positive or negative)
 */
void updatePosition(long steps);

/**
 * Get the current absolute position
 * 
 * @return Current position in steps
 */
long getCurrentPosition();

/**
 * Set the current position to a specific value
 * Useful for zeroing or reference setting
 * 
 * @param position New position value
 */
void setCurrentPosition(long position);

/**
 * Get the minimum position (CW limit)
 * 
 * @return Minimum position in steps
 */
long getMinPosition();

/**
 * Set the minimum position (CW limit)
 * 
 * @param position New minimum position
 */
void setMinPosition(long position);

/**
 * Get the maximum position (CCW limit)
 * 
 * @return Maximum position in steps
 */
long getMaxPosition();

/**
 * Set the maximum position (CCW limit)
 * 
 * @param position New maximum position
 */
void setMaxPosition(long position);

/**
 * Check if the limits have been initialized
 * 
 * @return TRUE if limits are initialized, FALSE otherwise
 */
bool areLimitsInitialized();

/**
 * Set the initialized state of the limits
 * 
 * @param initialized TRUE if limits are initialized, FALSE otherwise
 */
void setLimitsInitialized(bool initialized);

/**
 * Get the calibration mode state
 * 
 * @return TRUE if in calibration mode, FALSE otherwise
 */
bool isCalibrationMode();

/**
 * Set the calibration mode state
 * 
 * @param mode TRUE to enable calibration mode, FALSE to disable
 */
void setCalibrationMode(bool mode);

/**
 * Check if a target position is within the valid range
 * 
 * @param position Position to check
 * @return TRUE if position is within limits, FALSE otherwise
 */
bool isPositionInRange(long position);

/**
 * Constrain a position to be within valid limits
 * 
 * @param position Position to constrain
 * @return The constrained position value
 */
long constrainPosition(long position);

/**
 * Calculate percentage of travel (0-100%)
 * 
 * @return Percentage between min and max position
 */
int getPositionPercentage();

/**
 * Reset the position to zero
 * For backward compatibility
 */
void resetPosition();

#endif // POSITION_MANAGER_