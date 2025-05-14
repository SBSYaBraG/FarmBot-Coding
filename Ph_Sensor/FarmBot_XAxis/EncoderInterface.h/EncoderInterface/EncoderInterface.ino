/**
 * EncoderInterface.h
 * 
 * Header file for encoder reading and position tracking.
 * Handles quadrature encoder signals for position verification.
 */

#ifndef ENCODER_INTERFACE_H
#define ENCODER_INTERFACE_H

#include <Arduino.h>

/**
 * Initialize the encoder interface
 * Sets up pins and attaches interrupts
 */
void initializeEncoder();

/**
 * Read encoder signals and update position
 * Called by interrupt when encoder signals change
 */
void readEncoder();

/**
 * Get the current encoder position count
 * 
 * @return Current encoder position
 */
long getEncoderPosition();

/**
 * Reset encoder position to zero
 * Useful when homing or calibrating
 */
void resetEncoderPosition();

/**
 * Print encoder and step counter positions
 * For debugging and status reporting
 */
void printEncoderPosition();