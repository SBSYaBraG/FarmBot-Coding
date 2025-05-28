/**
 * CommandProcessor.h
 * 
 * Header file for serial command processing in the simplified FarmBot X-Axis controller.
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
 * Report the current system status
 * Prints position and other important information
 */
void reportStatus();

#endif // COMMAND_PROCESSOR_H