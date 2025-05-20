/**
 * PositionManager.h
 * 
 * Header file for position tracking in the simplified FarmBot X-Axis controller.
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
 * Get the maximum position limit
 * 
 * @return Maximum position in steps
 */
long getMaxPosition();

/**
 * Set the maximum position limit
 * This is updated if the far limit switch is triggered
 * 
 * @param position New maximum position
 */
void setMaxPosition(long position);

/**
 * Calculate percentage of travel (0-100%)
 * 
 * @return Percentage between 0 and max position
 */
int getPositionPercentage();

#endif // POSITION_MANAGER_H
