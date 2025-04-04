//FarmBot Stepper Motor Control Using TB6600 with Rotary and Limit Switches Practice

//Stepper Motor pins
#define DIR 5 //DIR+ from TB6600 to Digital Pin 5 or Arduino
#define PUL 4 //PUL+ from TB6600 to Digital Pin 4 or Arduino

#define Steps_Rev 3200 //Number of Steps/Rev (More steps = more precision = Less Torque!)

//Microsecond delay between pulses (Steps/s = 1/Step_Delay = Frequency of Pulses)
#define Min_Step_Delay 200  //Fastest Speed
#define Max_Step_Delay 1200 // Slowest Speed
#define Acceleration_rate 10 //Amount to change step delay--per step
//Acceleration prevents skipping steps, mechanical stress, and smoothens motion and control
#define Deceleration_rate 10 //Same as acceleration rate for simplicity
//In order to not stop abruptly we decelerate when we start getting close to the destination


//Rotary Encoder Pins
#define Encoder_A 2 //CLK pin from rotary encoder
#define Encoder_B 3 //DT (Data) pin from encoder


//Limit Switches Pins (Normally Open Config, Pulled HIGH by INPUT_PULLUP)
#define LIMIT_X 7
#define LIMIT_Y 8
#define LIMIT_Z 9


//Global Variables (only for encoder tracking so far)
volatile long encoderPosition = 0; //Store Encoder Position
//Store the last known states of the encodes pins
volatile bool lastA = LOW;
volatile bool lastB = LOW;

//Flag to check if limit switch is triggered 
bool limitTriggered = false;

void setup() {
  
  //Set Motor Pins
  pinMode(DIR, OUTPUT); //Set direction pin as OUTPUT
  pinMode(PUL, OUTPUT); //Set pulse pin as OUTPUT

  
  //Set Encoder Pins
  pinMode(Encoder_A, INPUT_PULLUP); //Keeps signal HIGH until connected to GND
  pinMode(Encoder_B, INPUT_PULLUP);

  
  //Set Limit Switch Pins
  pinMode(LIMIT_X, INPUT_PULLUP); //Normally HIGH, pulled LOW when pressed
  pinMode(LIMIT_Y, INPUT_PULLUP); 
  pinMode(LIMIT_Z, INPUT_PULLUP); 
  
  
  //Attach interrupts for encoder channels, it triggers the 'readEncoder' function whenever a change occurs
  attachInterrupt(digitalPinToInterrupt(Encoder_A), readEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(Encoder_B), readEncoder, CHANGE);
  

  Serial.begin(115200);
  Serial.println("System Initialized. Starting Homing Routine...");
}

void loop() {
  
  //Homing each axis separately before starting normal operation
  homeAxis(LIMIT_X, true);
  homeAxis(LIMIT_Y, true);
  homeAxis(LIMIT_Z, true);
  Serial.println("Hominh Completed. Ready for Normal Operation.");

  //Check limit switch status before each movement
  checkLimitSwitches();

  //Example: Move up to 1 Rev if no limit switch is triggered
  if(!limitTriggered){
    moveToPosition(3200);
  delay(1000); 

  //Move back to start position (1 Rev, opposite DIR)
  moveToPosition(0);
  delay(1000);
  }
  else{
    Serial.println("Movement Halted. Limit Switch is Triggered!");
    delay(1000);
  }
  
}

void moveToPosition(long targetPosition){
  long stepsToMove = targetPosition - encoderPosition;
  bool direction = (stepsToMove > 0);
  digitalWrite(DIR, direction ? HIGH:LOW);

  int stepDelay = Max_Step_Delay; //Start From Slowest Speed

  //Accelerate
  while(abs(stepsToMove) > Steps_Rev/4 && abs(stepsToMove) > 0){//Accelerate the first 1/4 of the journey
      stepMotor(stepDelay);

      //update the number of steps to move
      stepsToMove = targetPosition - encoderPosition;

      //Speed up the motor
      if(stepDelay > Min_Step_Delay){
        stepDelay -= Acceleration_rate;
        if(stepDelay < Min_Step_Delay) stepDelay = Min_Step_Delay;
      }
  }

  //Crusing Phase
  while(abs(stepsToMove) > Steps_Rev/4){//Moving at Constant Speed
    stepMotor(stepDelay);

    //Update the number of steps to move
    stepsToMove = targetPosition - encoderPosition;
  }

  //Decelaration Phase
  while(abs(stepsToMove) > 0){//Gradually slow down until reaching the destination
    stepMotor(stepDelay);

    //Update number of steps to move
    stepsToMove = targetPosition - encoderPosition;

    //Slow down the motor
    if(stepDelay < Max_Step_Delay){
      stepDelay += Deceleration_rate;
      if(stepDelay > Max_Step_Delay) stepDelay = Max_Step_Delay;
    }
  }
  Serial.println("Target Position Reached! (Hopefully :3)");
}


void stepMotor(int delayTime){
    digitalWrite(PUL, HIGH); //Send Pulse
    delayMicroseconds(delayTime);
    digitalWrite(PUL, LOW); //End Pulse
    delayMicroseconds(delayTime);
 }

/* Basic Acceleration/Deceleration Movement
void rotateStepper(int totalSteps, bool clockwise) {
  int stepDelay = Max_Step_Delay; //Start from slowest speed
  digitalWrite(DIR, clockwise ? HIGH:LOW);

  int accelSteps = totalSteps/4; //Accelerate over the first 1/4 of the journey
  int decelSteps = totalSteps/4; //Decelerate over the last 1/4 of the journey
  int cruiseSteps = totalSteps - accelSteps - decelSteps; //Constant speed in middle of the journey

  //Acceleration Phase
  for(int i=0; i<accelSteps; i++)
  {
    stepMotor(stepDelay);
    //Accelerate (reduce step delay) until reaching Min_Step_Delay
    if(stepDelay > Min_Step_Delay){
      stepDelay -= Acceleration_rate;
      if(stepDelay < Min_Step_Delay) stepDelay = Min_Step_Delay;
    }

  //Cruising Phase
  for(int i=0; i<cruiseSteps; i++){
    stepMotor(stepDelay);
  }

  //Deceleration Phase
  for(int i=0; i<decelSteps; i++){
    stepMotor(stepDelay);
    //Decelerate (Increase step delay) until reaching Max_Step_Delay
    if(stepDelay < Max_Step_Delay){
      stepDelay += Deceleration_rate;
      if(stepDelay > Max_Step_Delay) stepDelay = Max_Step_Delay;
    }
  }
  
  }
}
*/
void readEncoder(){
  //Read Current State of Encoder pins
  bool currentA = digitalRead(Encoder_A);
  bool currentB = digitalRead(Encoder_B);

  //Check if state of A or B has changed
  if(currentA == lastA && currentB == lastB) return; //No change, do nothing

  //Determine the direction of rotation
  if(currentA != lastA){ //Channel A changed state

    if(currentA == currentB) encoderPosition++; //CW (CLK=DT, no lags or leads)
    else encoderPosition--;                     //CCW (CLK != DT, CLK either leads or lags DT)
  }
  else if(currentB != lastB){ //Channel B changed state, catches everymovement even if noisy or too fast
    
    if(currentA != currentB) encoderPosition++; //CW
    else encoderPosition--;                     //CCW
  }

  //Store the last states for comparison in the next interrupt call
  lastA = currentA;
  lastB = currentB;
}


void checkLimitSwitches(){
  
  if(digitalRead(LIMIT_X) == LOW){
    limitTriggered = true;
    Serial.println("X-Axis Limit Switch Triggered!");
  }
  else if(digitalRead(LIMIT_Y) == LOW){
    limitTriggered = true;
    Serial.println("Y-Axis Limit Switch Triggered!");
  }
  else if(digitalRead(LIMIT_Z) == LOW){
    limitTriggered = true;
    Serial.println("Z-Axis Limit Switch Triggered!");
  }
  else{
    limitTriggered = false; //no limit switch is pressed
  }
}

//HOMING FUNCTIon
void homeAxis(int limitSwitchPin, bool direction){
  Serial.print("Homing Axis with Limit Switch: ");
  Serial.println(limitSwitchPin);

  digitalWrite(DIR, direction ? HIGH:LOW);
  int stepDelay = Max_Step_Delay;

  //Move Towards the limit switch until triggered
  while(digitalRead(limitSwitchPin) == HIGH){
    stepMotor(stepDelay);

    //Check for limit switch activation
    if(digitalRead(limitSwitchPin) == LOW){
      Serial.println("Limit Switch Triggered. Stopping Movement on Axis.");
      encoderPosition = 0; //Reset Encoder Count as reference point
      delay(500); 
      break;
    }
  }
  Serial.println("Axis Home Succesfully.");
}
