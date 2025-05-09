v// ------------------------------
// FarmBot X-Axis Motor Control with Absolute Positioning and Safe Limit Recovery
// ------------------------------

// --- Pin Assignments ---
#define DIR 5     // Motor direction pin (HIGH = CCW, LOW = CW)
#define PUL 4     // Step pin to pulse motor
#define ENA 6     // Enable pin (LOW = enabled, HIGH = disabled)

// --- Motor Parameters ---
#define Steps_Rev 3200           // Steps per full revolution (used for context/reference)
#define Min_Step_Delay 200       // Smallest delay between steps (fastest speed)
#define Max_Step_Delay 1200      // Largest delay between steps (slowest speed)
#define Acceleration_rate 10     // How much delay decreases during acceleration
#define Deceleration_rate 10     // How much delay increases during deceleration

// --- Encoder and Limit Switch Pins ---
#define Encoder_A 2              // Encoder channel A
#define Encoder_B 3              // Encoder channel B
#define LIMIT_X 7                // Limit switch on X-axis
#define BACKOFF_STEPS 200        // Steps to back off after hitting a limit

// --- EEPROM Addressing (Non-volatile memory for limits) ---
#define EEPROM_INITIALIZED_ADDR 0
#define EEPROM_MIN_POS_ADDR 4
#define EEPROM_MAX_POS_ADDR 8
#define EEPROM_MAGIC_NUMBER 42   // Magic number to verify EEPROM is valid

// --- State Tracking Variables ---
volatile long encoderPosition = 0;     // Tracks rotary encoder counts
volatile bool lastA = LOW, lastB = LOW;

long currentPosition = 0;              // Current absolute step position
long minPosition = 0;                  // Calibrated minimum step limit
long maxPosition = 0;                  // Calibrated maximum step limit
bool limitsInitialized = false;        // True if EEPROM or calibration has provided limits

// --- System Mode Flags ---
bool calibrationMode = false;          // Flag set only during calibration routine
bool homingMode = false;               // Flag set only during homing

unsigned long lastPrintTime = 0;       // Tracks last encoder print time to avoid flooding

// ------------------------------
// Setup runs once on boot
// ------------------------------
void setup() {
  Serial.begin(115200);  // Start serial monitor

  // Initialize motor control pins
  pinMode(DIR, OUTPUT);
  pinMode(PUL, OUTPUT);
  pinMode(ENA, OUTPUT);
  digitalWrite(ENA, HIGH); // Disable motor by default

  // Setup encoder pins and interrupts
  pinMode(Encoder_A, INPUT_PULLUP);
  pinMode(Encoder_B, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(Encoder_A), readEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(Encoder_B), readEncoder, CHANGE);

  // Setup limit switch
  pinMode(LIMIT_X, INPUT_PULLUP);

  // Load previous limits from EEPROM
  if (loadAxisLimits()) {
    Serial.println("Axis limits loaded from EEPROM");
    Serial.print("Min position: "); Serial.print(minPosition);
    Serial.print(" | Max position: "); Serial.println(maxPosition);
    limitsInitialized = true;
  } else {
    Serial.println("No axis limits found. Run calibration first.");
  }

  // Show command menu
  Serial.println("System Ready. Commands:");
  Serial.println("  X#### or X-#### - Move to relative position");
  Serial.println("  P#### - Move to absolute position");
  Serial.println("  C - Run calibration");
  Serial.println("  H - Run homing sequence");
  Serial.println("  R - Report current position and limits");
}

// ------------------------------
// Main loop: check for serial commands
// ------------------------------
void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();  // Clean trailing spaces

    if (input.startsWith("X")) {
      processRelativeMove(input.substring(1).toInt());  // Move relative
    }
    else if (input.startsWith("P")) {
      processAbsoluteMove(input.substring(1).toInt());  // Move absolute
    }
    else if (input.startsWith("C")) {
      runCalibration();  // Start calibration routine
    }
    else if (input.startsWith("H")) {
      runHoming();  // Start homing routine
    }
    else if (input.startsWith("R")) {
      reportStatus();  // Print system status
    }
  }
}

// ------------------------------
// Move relative to current position
// ------------------------------
void processRelativeMove(long steps) {
  if (steps == 0) return;  // No movement

  Serial.print("Relative move: "); Serial.println(steps);
  long targetPosition = currentPosition + steps;

  // Check against limits if calibrated
  if (limitsInitialized) {
    if (targetPosition < minPosition) {
      Serial.println("Move would exceed minimum limit. Moving to min position.");
      targetPosition = minPosition;
    }
    else if (targetPosition > maxPosition) {
      Serial.println("Move would exceed maximum limit. Moving to max position.");
      targetPosition = maxPosition;
    }
  }

  // Recalculate steps after limiting
  steps = targetPosition - currentPosition;
  bool direction = (steps > 0);  // HIGH = CCW, LOW = CW

  if (steps != 0) {
    digitalWrite(DIR, direction ? HIGH : LOW);
    digitalWrite(ENA, LOW);  // Enable motor
    moveSteps(abs(steps), direction);  // Execute move
    digitalWrite(ENA, HIGH); // Disable motor
    currentPosition = targetPosition;
    Serial.print("Move complete. Current position: "); Serial.println(currentPosition);
  }
}

// ------------------------------
// Move to a specific absolute position
// ------------------------------
void processAbsoluteMove(long targetPosition) {
  if (!limitsInitialized) {
    Serial.println("Cannot move to absolute position: Limits not calibrated");
    return;
  }

  Serial.print("Moving to absolute position: "); Serial.println(targetPosition);

  // Clamp within bounds
  if (targetPosition < minPosition) {
    Serial.println("Target below minimum. Moving to min position.");
    targetPosition = minPosition;
  }
  else if (targetPosition > maxPosition) {
    Serial.println("Target above maximum. Moving to max position.");
    targetPosition = maxPosition;
  }

  long steps = targetPosition - currentPosition;
  bool direction = (steps > 0);

  if (steps != 0) {
    digitalWrite(DIR, direction ? HIGH : LOW);
    digitalWrite(ENA, LOW);
    moveSteps(abs(steps), direction);
    digitalWrite(ENA, HIGH);
    currentPosition = targetPosition;
    Serial.print("Move complete. Current position: "); Serial.println(currentPosition);
  } else {
    Serial.println("Already at target position.");
  }
}

// ------------------------------
// Move specified number of steps with acceleration profile
// ------------------------------
void moveSteps(long stepsToMove, bool direction) {
  int stepDelay = Max_Step_Delay;
  long totalSteps = stepsToMove;

  // Motion profile: 25% accel, 50% constant, 25% decel
  long accelerationSteps = totalSteps / 4;
  long decelerationSteps = totalSteps / 4;
  long constantSteps = totalSteps - accelerationSteps - decelerationSteps;

  // Accelerate
  for (long i = 0; i < accelerationSteps; i++) {
    if (checkLimitSwitch(direction)) return;
    stepMotor(stepDelay, direction);
    stepDelay = max(stepDelay - Acceleration_rate, Min_Step_Delay);
    printEncoderPosition();
  }

  // Constant speed
  for (long i = 0; i < constantSteps; i++) {
    if (checkLimitSwitch(direction)) return;
    stepMotor(stepDelay, direction);
    printEncoderPosition();
  }

  // Decelerate
  for (long i = 0; i < decelerationSteps; i++) {
    if (checkLimitSwitch(direction)) return;
    stepMotor(stepDelay, direction);
    stepDelay = min(stepDelay + Deceleration_rate, Max_Step_Delay);
    printEncoderPosition();
  }
}

// ------------------------------
// Send one pulse to stepper motor
// ------------------------------
void stepMotor(int delayTime, bool direction) {
  digitalWrite(PUL, HIGH);
  delayMicroseconds(10);
  digitalWrite(PUL, LOW);
  delayMicroseconds(delayTime - 10);

  // Update internal position
  currentPosition += (direction ? 1 : -1);
}

// ------------------------------
// Encoder interrupt routine: detects direction and counts steps
// ------------------------------
void readEncoder() {
  bool currentA = digitalRead(Encoder_A);
  bool currentB = digitalRead(Encoder_B);

  if (currentA == lastA && currentB == lastB) return;

  if (currentA != lastA) {
    encoderPosition += (currentA == currentB) ? 1 : -1;
  } else if (currentB != lastB) {
    encoderPosition += (currentA != currentB) ? 1 : -1;
  }

  lastA = currentA;
  lastB = currentB;
}

// ------------------------------
// Print encoder and motor positions every 100 ms
// ------------------------------
void printEncoderPosition() {
  unsigned long now = millis();
  if (now - lastPrintTime >= 100) {
    Serial.print("Encoder: "); Serial.print(encoderPosition);
    Serial.print(" | Position: "); Serial.println(currentPosition);
    lastPrintTime = now;
  }
}

// ------------------------------
// Limit switch handler during movement
// ------------------------------
bool checkLimitSwitch(bool direction) {
  if (digitalRead(LIMIT_X) == LOW) {
    Serial.println("LIMIT SWITCH PRESSED!");

    if (calibrationMode) {
      if (!direction) {
        minPosition = currentPosition;
        Serial.print("Minimum position set: "); Serial.println(minPosition);
      } else {
        maxPosition = currentPosition;
        Serial.print("Maximum position set: "); Serial.println(maxPosition);
      }
    } else {
      Serial.println("Backing off...");

      // Snap to limit and back off a little
      currentPosition = direction ? maxPosition : minPosition;
      digitalWrite(DIR, !direction);
      delay(50);

      for (int i = 0; i < BACKOFF_STEPS; i++) {
        stepMotor(Max_Step_Delay, !direction);
      }

      Serial.println("Backed off from limit.");
    }

    return true;  // Stop current motion
  }

  return false;
}

// ------------------------------
// Calibration routine: finds both limits
// ------------------------------
void runCalibration() {
  Serial.println("STARTING CALIBRATION");
  calibrationMode = true;
  currentPosition = 0;

  // Move CW to find min limit
  Serial.println("Moving to find CW (minimum) limit...");
  digitalWrite(DIR, LOW);
  digitalWrite(ENA, LOW);
  while (digitalRead(LIMIT_X) == HIGH) {
    stepMotor(Max_Step_Delay, false);
  }

  // Back off from limit
  digitalWrite(DIR, HIGH);
  for (int i = 0; i < BACKOFF_STEPS; i++) {
    stepMotor(Max_Step_Delay, true);
  }

  minPosition = currentPosition;
  Serial.print("Minimum position set to: "); Serial.println(minPosition);

  // Move CCW to find max limit
  Serial.println("Moving to find CCW (maximum) limit...");
  digitalWrite(DIR, HIGH);
  while (digitalRead(LIMIT_X) == HIGH) {
    stepMotor(Max_Step_Delay, true);
  }

  // Back off again
  digitalWrite(DIR, LOW);
  for (int i = 0; i < BACKOFF_STEPS; i++) {
    stepMotor(Max_Step_Delay, false);
  }

  maxPosition = currentPosition;
  Serial.print("Maximum position set to: "); Serial.println(maxPosition);

  // Save and center
  saveAxisLimits();
  calibrationMode = false;
  limitsInitialized = true;

  long center = minPosition + (maxPosition - minPosition) / 2;
  processAbsoluteMove(center);

  Serial.println("CALIBRATION COMPLETE");
}

// ------------------------------
// Homing routine: return to minimum limit
// ------------------------------
void runHoming() {
  if (!limitsInitialized) {
    Serial.println("Cannot home: Limits not calibrated");
    return;
  }

  Serial.println("HOMING SEQUENCE STARTED");
  homingMode = true;
  processAbsoluteMove(minPosition + BACKOFF_STEPS);
  Serial.println("X-axis homed successfully");
  homingMode = false;
}

// ------------------------------
// System report: print all key parameters
// ------------------------------
void reportStatus() {
  Serial.println("\n----- SYSTEM STATUS -----");
  Serial.print("Current position: "); Serial.println(currentPosition);

  if (limitsInitialized) {
    Serial.print("Minimum position: "); Serial.println(minPosition);
    Serial.print("Maximum position: "); Serial.println(maxPosition);
    Serial.print("Total travel range: "); Serial.println(maxPosition - minPosition);

    int percent = map(currentPosition, minPosition, maxPosition, 0, 100);
    Serial.print("Current position %: "); Serial.print(percent); Serial.println("%");
  } else {
    Serial.println("Axis limits not calibrated");
  }

  Serial.print("Encoder position: "); Serial.println(encoderPosition);
  Serial.println("------------------------\n");
}

// ------------------------------
// EEPROM Utility: Save calibrated limits
// ------------------------------
#include <EEPROM.h>

void saveAxisLimits() {
  EEPROM.put(EEPROM_INITIALIZED_ADDR, EEPROM_MAGIC_NUMBER);
  EEPROM.put(EEPROM_MIN_POS_ADDR, minPosition);
  EEPROM.put(EEPROM_MAX_POS_ADDR, maxPosition);
  Serial.println("Axis limits saved to EEPROM");
}

// ------------------------------
// EEPROM Utility: Load saved limits if valid
// ------------------------------
bool loadAxisLimits() {
  int magic;
  EEPROM.get(EEPROM_INITIALIZED_ADDR, magic);
  if (magic == EEPROM_MAGIC_NUMBER) {
    EEPROM.get(EEPROM_MIN_POS_ADDR, minPosition);
    EEPROM.get(EEPROM_MAX_POS_ADDR, maxPosition);
    return true;
  }
  return false;
}
