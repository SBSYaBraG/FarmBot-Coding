/**
 * StorageManager.h
 * 
 * Header file for EEPROM storage operations.
 * Handles saving and loading axis limits to persistent storage.
 */

#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <Arduino.h>

/**
 * Save axis limits to EEPROM
 * Stores the current minimum and maximum positions for persistence
 * 
 * @return TRUE if save was successful, FALSE otherwise
 */
bool saveAxisLimits();

/**
 * Load axis limits from EEPROM
 * Retrieves previously stored minimum and maximum positions
 * 
 * @return TRUE if valid limits were loaded, FALSE otherwise
 */
bool loadAxisLimits();

/**
 * Reset stored limits in EEPROM
 * Clears any saved calibration data
 */
void resetStoredLimits();

#endif // STORAGE_MANAGER_H