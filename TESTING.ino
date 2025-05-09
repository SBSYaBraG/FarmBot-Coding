// FarmBot X-Axis Motor Control with Absolute Positioning and Safe Limit Recovery

#define DIR 5
#define PUL 4
#define ENA 6

#define Steps_Rev 3200
#define Min_Step_Delay 200
#define Max_Step_Delay 1200
#define Acceleration_rate 10
#define Deceleration_rate 10

#define Encoder_A 2
#define Encoder_B 3
#define LIMIT_X 7
#define BACKOFF_STEPS 200

// EEPROM addresses for storing axis limits
#define EEPROM_INITIALIZED_ADDR 0
#define EEPROM_MIN_POS_ADDR 4
#define EEPROM_MAX_POS_ADDR 8
#define EEPROM_MAGIC_NUMBER 42  // To verify EEPROM was initialized

// Track encoder and motor step counts
volatile long encoderPosition = 0;
volatile bool lastA = LOW;
volatile bool lastB = LOW;

long currentPosition = 0;    // Current absolute position
long minPosition = 0;        // Minimum position (CW limit)
long maxPosition = 0;        // Maximum position (CCW limit)
bool limitsInitialized = false;

// System state
bool calibrationMode = false;
bool homingMode = false;

unsigned long lastPrintTime = 0;

void setup() {
  Serial.begin(115200);
  pinMode(DIR, OUTPUT);
  pinMode(PUL, OUTPUT);
  pinMode(ENA, OUTPUT);
  digitalWrite(ENA, HIGH); // Motor disabled

  pinMode(Encoder_A, INPUT_PULLUP);
  pinMode(Encoder_B, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(Encoder_A), readEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(Encoder_B), readEncoder, CHANGE);

  pinMode(LIMIT_X, INPUT_PULLUP);

  // Try to read axis limits from EEPROM
  if (loadAxisLimits()) {
    Serial.println("Axis limits loaded from EEPROM");
    Serial.print("Min position: ");
    Serial.print(minPosition);
    Serial.print(" | Max position: ");
    Serial.println(maxPosition);
    limitsInitialized = true;
  } else {
    Serial.println("No axis limits found. Run calibration first.");
  }

  Serial.println("System Ready. Commands:");
  Serial.println("  X#### or X-#### - Move to relative position");
  Serial.println("  P#### - Move to absolute position");
  Serial.println("  C - Run calibration");
  Serial.println("  H - Run homing sequence");
  Serial.println("  R - Report current position and limits");
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    // Command processing
    if (input.startsWith("X")) {
      // Relative movement command
      long steps = input.substring(1).toInt();
      processRelativeMove(steps);
    }
    else if (input.startsWith("P")) {
      // Absolute positioning command
      long targetPos = input.substring(1).toInt();
      processAbsoluteMove(targetPos);
    }
    else if (input.startsWith("C")) {
      // Calibration command
      runCalibration();
    }
    else if (input.startsWith("H")) {
      // Homing command
      runHoming();
    }
    else if (input.startsWith("R")) {
      // Report status
      reportStatus();
    }
  }
}

void processRelativeMove(long steps) {
  if (steps == 0) return;
  
  Serial.print("Relative move: ");
  Serial.println(steps);
  
  // Calculate target position
  long targetPosition = currentPosition + steps;
  
  // Check if the move would exceed limits
  if (limitsInitialized) {
    if (targetPosition < minPosition) {
      Serial.println("Move would exceed minimum limit. Moving to min position instead.");
      targetPosition = minPosition;
    }
    else if (targetPosition > maxPosition) {
      Serial.println("Move would exceed maximum limit. Moving to max position instead.");
      targetPosition = maxPosition;
    }
  }
  
  // Calculate actual steps needed
  steps = targetPosition - currentPosition;
  bool direction = (steps > 0);  // TRUE = CCW (HIGH), FALSE = CW (LOW)
  
  if (steps != 0) {
    digitalWrite(DIR, direction ? HIGH : LOW);
    digitalWrite(ENA, LOW);
    moveSteps(abs(steps), direction);
    digitalWrite(ENA, HIGH);
    
    // Update position
    currentPosition = targetPosition;
    Serial.print("Move complete. Current position: ");
    Serial.println(currentPosition);
  }
}

void processAbsoluteMove(long targetPosition) {
  if (!limitsInitialized) {
    Serial.println("Cannot move to absolute position: Limits not calibrated");
    return;
  }
  
  Serial.print("Moving to absolute position: ");
  Serial.println(targetPosition);
  
  // Constrain target position to valid range
  if (targetPosition < minPosition) {
    Serial.println("Target below minimum. Moving to min position.");
    targetPosition = minPosition;
  }
  else if (targetPosition > maxPosition) {
    Serial.println("Target above maximum. Moving to max position.");
    targetPosition = maxPosition;
  }
  
  // Calculate steps needed
  long steps = targetPosition - currentPosition;
  bool direction = (steps > 0);  // TRUE = CCW (HIGH), FALSE = CW (LOW)
  
  if (steps != 0) {
    digitalWrite(DIR, direction ? HIGH : LOW);
    digitalWrite(ENA, LOW);
    moveSteps(abs(steps), direction);
    digitalWrite(ENA, HIGH);
    
    // Update position
    currentPosition = targetPosition;
    Serial.print("Move complete. Current position: ");
    Serial.println(currentPosition);
  } else {
    Serial.println("Already at target position.");
  }
}

void moveSteps(long stepsToMove, bool direction) {
  int stepDelay = Max_Step_Delay;
  long totalSteps = stepsToMove;
  long accelerationSteps = totalSteps / 4;  // Use 1/4 of total steps for acceleration
  long decelerationSteps = totalSteps / 4;  // Use 1/4 of total steps for deceleration
  long constantSteps = totalSteps - accelerationSteps - decelerationSteps;

  // Acceleration phase
  for (long i = 0; i < accelerationSteps; i++) {
    if (checkLimitSwitch(direction)) return;
    stepMotor(stepDelay, direction);
    
    if (stepDelay > Min_Step_Delay) {
      stepDelay -= Acceleration_rate;
      if (stepDelay < Min_Step_Delay) stepDelay = Min_Step_Delay;
    }
    printEncoderPosition();
  }

  // Constant speed phase
  for (long i = 0; i < constantSteps; i++) {
    if (checkLimitSwitch(direction)) return;
    stepMotor(stepDelay, direction);
    printEncoderPosition();
  }

  // Deceleration phase
  for (long i = 0; i < decelerationSteps; i++) {
    if (checkLimitSwitch(direction)) return;
    stepMotor(stepDelay, direction);
    
    if (stepDelay < Max_Step_Delay) {
      stepDelay += Deceleration_rate;
      if (stepDelay > Max_Step_Delay) stepDelay = Max_Step_Delay;
    }
    printEncoderPosition();
  }
}

void stepMotor(int delayTime, bool direction) {
  digitalWrite(PUL, HIGH);
  delayMicroseconds(10);
  digitalWrite(PUL, LOW);
  delayMicroseconds(delayTime - 10);

  // Update position based on direction
  currentPosition += (direction ? 1 : -1);
}

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

void printEncoderPosition() {
  unsigned long now = millis();
  if (now - lastPrintTime >= 100) {
    Serial.print("Encoder: ");
    Serial.print(encoderPosition);
    Serial.print(" | Position: ");
    Serial.println(currentPosition);
    lastPrintTime = now;
  }
}

bool checkLimitSwitch(bool direction) {
  if (digitalRead(LIMIT_X) == LOW) {
    Serial.println("LIMIT SWITCH PRESSED!");
    
    // Record the position as a limit if in calibration mode
    if (calibrationMode) {
      if (!direction) { // CW -> minimum position
        minPosition = currentPosition;
        Serial.print("Minimum position set: ");
        Serial.println(minPosition);
      } else { // CCW -> maximum position
        maxPosition = currentPosition;
        Serial.print("Maximum position set: ");
        Serial.println(maxPosition);
      }
    } else {
      // Normal operation - back off from limit
      Serial.println("Backing off...");
      
      // Update current position to match limit if hit
      if (!direction) { // CW -> we hit the minimum
        currentPosition = minPosition;
      } else { // CCW -> we hit the maximum
        currentPosition = maxPosition;
      }
      
      // Back off in opposite direction
      digitalWrite(DIR, !direction);
      delay(50);
      
      for (int i = 0; i < BACKOFF_STEPS; i++) {
        stepMotor(Max_Step_Delay, !direction);
      }
      
      Serial.println("Backed off from limit.");
    }
    return true;
  }
  return false;
}

void runCalibration() {
  Serial.println("STARTING CALIBRATION");
  Serial.println("Moving to find CW (minimum) limit...");
  
  calibrationMode = true;
  currentPosition = 0; // Reset position counter
  
  // First move CW until we hit the limit
  digitalWrite(DIR, LOW); // CW direction
  digitalWrite(ENA, LOW);
  
  while (digitalRead(LIMIT_X) == HIGH) {
    stepMotor(Max_Step_Delay, false);
  }
  
  Serial.println("CW limit found!");
  
  // Back off from the limit
  digitalWrite(DIR, HIGH);
  for (int i = 0; i < BACKOFF_STEPS; i++) {
    stepMotor(Max_Step_Delay, true);
  }
  
  // Set this as position 0
  minPosition = currentPosition;
  Serial.print("Minimum position set to: ");
  Serial.println(minPosition);
  
  // Now move CCW until we hit the other limit
  Serial.println("Moving to find CCW (maximum) limit...");
  digitalWrite(DIR, HIGH); // CCW direction
  
  while (digitalRead(LIMIT_X) == HIGH) {
    stepMotor(Max_Step_Delay, true);
  }
  
  Serial.println("CCW limit found!");
  
  // Back off from the limit
  digitalWrite(DIR, LOW);
  for (int i = 0; i < BACKOFF_STEPS; i++) {
    stepMotor(Max_Step_Delay, false);
  }
  
  // Record the maximum position
  maxPosition = currentPosition;
  Serial.print("Maximum position set to: ");
  Serial.println(maxPosition);
  
  // Calculate travel distance
  long travelDistance = maxPosition - minPosition;
  Serial.print("Total travel distance: ");
  Serial.print(travelDistance);
  Serial.println(" steps");
  
  // Save limits to EEPROM
  saveAxisLimits();
  
  calibrationMode = false;
  limitsInitialized = true;
  
  // Move to center position
  long centerPosition = minPosition + (travelDistance / 2);
  processAbsoluteMove(centerPosition);
  
  Serial.println("CALIBRATION COMPLETE");
}

void runHoming() {
  if (!limitsInitialized) {
    Serial.println("Cannot home: Limits not calibrated");
    return;
  }
  
  Serial.println("HOMING SEQUENCE STARTED");
  homingMode = true;
  
  // Move to minimum position (CW limit)
  Serial.println("Moving to home position (minimum limit)...");
  processAbsoluteMove(minPosition + BACKOFF_STEPS);
  
  // Now we're at a known position
  Serial.println("X-axis homed successfully");
  homingMode = false;
}

void reportStatus() {
  Serial.println("\n----- SYSTEM STATUS -----");
  Serial.print("Current position: ");
  Serial.println(currentPosition);
  
  if (limitsInitialized) {
    Serial.print("Minimum position: ");
    Serial.println(minPosition);
    Serial.print("Maximum position: ");
    Serial.println(maxPosition);
    Serial.print("Total travel range: ");
    Serial.print(maxPosition - minPosition);
    Serial.println(" steps");
    
    int percentPosition = map(currentPosition, minPosition, maxPosition, 0, 100);
    Serial.print("Current position %: ");
    Serial.print(percentPosition);
    Serial.println("%");
  } else {
    Serial.println("Axis limits not calibrated");
  }
  
  Serial.print("Encoder position: ");
  Serial.println(encoderPosition);
  Serial.println("------------------------\n");
}

// EEPROM functions for saving and loading axis limits
#include <EEPROM.h>

void saveAxisLimits() {
  EEPROM.put(EEPROM_INITIALIZED_ADDR, EEPROM_MAGIC_NUMBER);
  EEPROM.put(EEPROM_MIN_POS_ADDR, minPosition);
  EEPROM.put(EEPROM_MAX_POS_ADDR, maxPosition);
  Serial.println("Axis limits saved to EEPROM");
}

bool loadAxisLimits() {
  int magicNumber;
  EEPROM.get(EEPROM_INITIALIZED_ADDR, magicNumber);
  
  if (magicNumber == EEPROM_MAGIC_NUMBER) {
    EEPROM.get(EEPROM_MIN_POS_ADDR, minPosition);
    EEPROM.get(EEPROM_MAX_POS_ADDR, maxPosition);
    return true;
  }
  return false;
}