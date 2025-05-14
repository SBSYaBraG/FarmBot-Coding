/**
 * CommandProcessor.h
 * 
 * Header file for serial command processing.
 * Handles parsing and executing commands received via serial port.
 */

#ifndef COMMAND_PROCESSOR_H
#define COMMAND_PROCESSOR_H

#include <Arduino.h>

/**
 * Process a command string received from serial
 * Parses the command and calls the appropriate function
 * 
 * @param command The command string to process
 */
void processCommand(String command);

/**
 * Process a relative movement command
 * Format: "X####" for positive movement, "X-####" for negative
 * 
 * @param steps Number of steps to move (can be positive or negative)
 */
void processRelativeMove(long steps);

/**
 * Process an absolute positioning command
 * Format: "P####" to move to a specific position
 * 
 * @param targetPosition The absolute position to move to
 */
void processAbsoluteMove(long targetPosition);

/**
 * Report the current system status
 * Prints position, limits, and other status information
 */
void reportStatus();

#endif // COMMAND_PROCESSOR_H