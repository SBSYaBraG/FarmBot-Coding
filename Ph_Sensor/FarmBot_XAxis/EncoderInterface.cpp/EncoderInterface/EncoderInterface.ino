/**
 * EncoderInterface.cpp
 * 
 * Implementation of encoder reading and position tracking.
 * Handles quadrature encoder signals for position verification.
 */

#include "EncoderInterface.h"
#include "Config.h"
#include "PositionManager.h"

// Encoder state variables
volatile long encoderPosition = 0;
volatile bool lastA = LOW;
volatile bool lastB = LOW;
unsigned long lastPrintTime = 0;

/**
 * Initialize the encoder interface
 * Sets up pins and attaches interrupts
 */
void initializeEncoder() {
  // Set up encoder pins with pull-up resistors
  pinMode(ENCODER_A_PIN, INPUT_PULLUP);
  pinMode(ENCODER_B_PIN, INPUT_PULLUP);
  
  // Attach interrupt handlers to both encoder pins
  // When signal changes, the readEncoder function will be called
  attachInterrupt(digitalPinToInterrupt(ENCODER_A_PIN), readEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_B_PIN), readEncoder, CHANGE);
  
  // Initialize last states
  lastA = digitalRead(ENCODER_A_PIN);
  lastB = digitalRead(ENCODER_B_PIN);
}

/**
 * Read encoder signals and update position
 * Called by interrupt when encoder signals change
 * 
 * This function implements quadrature decoding to track position.
 * It detects both the direction and amount of movement based on
 * the sequence of signal changes from the two encoder channels.
 */
void readEncoder() {
  // Read current encoder channel states
  bool currentA = digitalRead(ENCODER_A_PIN);
  bool currentB = digitalRead(ENCODER_B_PIN);
  
  // If no change occurred (noise or bouncing), exit
  if (currentA == lastA && currentB == lastB) return;

  // Quadrature decoding logic
  if (currentA != lastA) {
    // A changed
    encoderPosition += (currentA == currentB) ? 1 : -1;
  } else if (currentB != lastB) {
    // B changed
    encoderPosition += (currentA != currentB) ? 1 : -1;
  }
  
  // Save current states for next comparison
  lastA = currentA;
  lastB = currentB;
}

/**
 * Get the current encoder position count
 * 
 * @return Current encoder position
 */
long getEncoderPosition() {
  return encoderPosition;
}

/**
 * Reset encoder position to zero
 * Useful when homing or calibrating
 */
void resetEncoderPosition() {
  encoderPosition = 0;
}

/**
 * Print encoder and step counter positions
 * For debugging and status reporting
 * 
 * This function throttles printing to once every 100ms
 * to avoid flooding the serial connection
 */
void printEncoderPosition() {
  unsigned long now = millis();
  
  // Only print every 100ms to avoid serial flooding
  if (now - lastPrintTime >= 100) {
    Serial.print("Encoder: ");
    Serial.print(encoderPosition);
    Serial.print(" | Position: ");
    Serial.println(getCurrentPosition());
    lastPrintTime = now;
  }
}