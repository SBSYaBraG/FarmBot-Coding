/**
 * MotorControl.h (Updated)
 * 
 * Header file for motor control functions with updated function signatures.
 */

#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>

/**
 * Initialize motor control pins
 */
void initializeMotor();

/**
 * Enable the motor (sets the enable pin to active state)
 */
void enableMotor();

/**
 * Disable the motor (power saving mode, no holding torque)
 */
void disableMotor();

/**
 * Set motor direction
 * 
 * @param direction TRUE for Counter-Clockwise (CCW, forward, HIGH pin state)
 *                  FALSE for Clockwise (CW, backward, LOW pin state)
 */
void setDirection(bool direction);

/**
 * Move a specified number of steps in the given direction with speed control.
 * Handles acceleration and deceleration smoothly.
 * 
 * @param stepsToMove Number of steps to move
 * @param direction TRUE for Counter-Clockwise (CCW), FALSE for Clockwise (CW)
 * @return TRUE if completed successfully, FALSE if stopped by limit switch
 */
bool moveSteps(long stepsToMove, bool direction);

/**
 * Generate a single step pulse
 * This advances the motor by one step in the current direction
 * 
 * @param delayTime Microsecond delay to wait after pulse (controls speed)
 * @param direction Direction of movement (used to update position counter)
 */
void stepMotor(int delayTime, bool direction);

/**
 * Emergency stop - immediately stop any movement
 */
void emergencyStop();

/**
 * Process a relative movement command with enhanced limit protection
 * 
 * @param steps Number of steps to move (can be positive or negative)
 */
void processRelativeMove(long steps);

#endif // MOTOR_CONTROL_H