/**
 * SystemOperations.h
 * 
 * Header file for system operations like calibration and homing.
 * Handles special sequences for initializing and maintaining the system.
 */

#ifndef SYSTEM_OPERATIONS_H
#define SYSTEM_OPERATIONS_H

#include <Arduino.h>

/**
 * Run the axis calibration routine
 * Finds both CW and CCW limits and sets up axis boundaries
 */
void runCalibration();

/**
 * Run the homing sequence
 * Returns to a known reference position (usually minimum position)
 */
void runHoming();

/**
 * Report the current system status (existing function in your code)
 * Prints position, limits, and other status information
 */
void reportStatus();

#endif // SYSTEM_OPERATIONS_H