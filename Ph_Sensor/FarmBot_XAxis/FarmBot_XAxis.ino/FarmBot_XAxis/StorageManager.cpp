/**
 * StorageManager.cpp
 * 
 * Implementation of EEPROM storage operations.
 * Handles saving and loading axis limits to persistent storage.
 */

#include "StorageManager.h"
#include "Config.h"
#include "PositionManager.h"
#include <EEPROM.h>

// Define EEPROM addresses for storing axis limits (for Arduino IDE compatibility)
#define EEPROM_INITIALIZED_ADDR 0  // Address to store initialization flag
#define EEPROM_MIN_POS_ADDR 4      // Address to store minimum position (4 bytes)
#define EEPROM_MAX_POS_ADDR 8      // Address to store maximum position (4 bytes)
#define EEPROM_MAGIC_NUMBER 42     // Value to verify EEPROM was initialized

/**
 * Save axis limits to EEPROM
 * Stores the current minimum and maximum positions for persistence
 * 
 * @return TRUE if save was successful, FALSE otherwise
 */
bool saveAxisLimits() {
  // Store a magic number to indicate valid data
  EEPROM.put(EEPROM_INITIALIZED_ADDR, EEPROM_MAGIC_NUMBER);
  
  // Store minimum position
  EEPROM.put(EEPROM_MIN_POS_ADDR, getMinPosition());
  
  // Store maximum position
  EEPROM.put(EEPROM_MAX_POS_ADDR, getMaxPosition());
  
  Serial.println("Axis limits saved to EEPROM");
  return true;
}

/**
 * Load axis limits from EEPROM
 * Retrieves previously stored minimum and maximum positions
 * 
 * @return TRUE if valid limits were loaded, FALSE otherwise
 */
bool loadAxisLimits() {
  // Check for initialization marker
  int magicNumber;
  EEPROM.get(EEPROM_INITIALIZED_ADDR, magicNumber);
  
  // If magic number matches, data is valid
  if (magicNumber == EEPROM_MAGIC_NUMBER) {
    // Read minimum position
    long minPos;
    EEPROM.get(EEPROM_MIN_POS_ADDR, minPos);
    setMinPosition(minPos);
    
    // Read maximum position
    long maxPos;
    EEPROM.get(EEPROM_MAX_POS_ADDR, maxPos);
    setMaxPosition(maxPos);
    
    return true;
  }
  
  // No valid data found
  return false;
}

/**
 * Reset stored limits in EEPROM
 * Clears any saved calibration data
 */
void resetStoredLimits() {
  // Write an invalid magic number to indicate no valid data
  EEPROM.put(EEPROM_INITIALIZED_ADDR, 0);
  Serial.println("Stored limits have been reset");
}