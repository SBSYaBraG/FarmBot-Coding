/**
 * LimitSwitch.h
 * 
 * Header file for limit switch detection in the simplified FarmBot X-Axis controller.
 */

#ifndef LIMIT_SWITCH_H
#define LIMIT_SWITCH_H

#include <Arduino.h>

/**
 * Initialize the limit switch pin
 */
void initializeLimitSwitch();

/**
 * Check if the limit switch is currently triggered
 * 
 * @param direction Current movement direction
 * @return TRUE if limit switch triggered and handled, FALSE otherwise
 */
bool checkLimitSwitch(bool direction);

/**
 * Back off from a triggered limit switch
 * 
 * @param direction Direction to back off (opposite of trigger direction)
 */
void backOffFromLimit(bool direction);

#endif // LIMIT_SWITCH_H