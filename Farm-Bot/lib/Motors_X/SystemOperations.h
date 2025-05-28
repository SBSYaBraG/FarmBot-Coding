/**
 * SystemOperations.h
 * 
 * Header file for system operations like homing in the simplified FarmBot X-Axis controller.
 */

#ifndef SYSTEM_OPERATIONS_H
#define SYSTEM_OPERATIONS_H

#include <Arduino.h>

/**
 * Run the homing sequence
 * Finds the home position (typically minimum position) and establishes
 * it as the zero reference point for all further movements
 */
void runHoming();

#endif // SYSTEM_OPERATIONS_H