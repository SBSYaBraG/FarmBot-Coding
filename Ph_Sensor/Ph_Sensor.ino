#define TdsSensorPin A1 //Analog Connection to Arduino (Input Signal)
#define VREF 5.0      // analog reference voltage(Volt) of the ADC (Standart Operation Voltage of Arduino)
#define SCOUNT  30           // # of samples collected from the Ph sensor for stability 
int analogBuffer[SCOUNT];    // store SCOUNT consecutive analog readings from the Ph sensor
int analogBufferTemp[SCOUNT]; // A copy of the readings later used to filter out noise (unexpected high or low readings)
int analogBufferIndex = 0; //Used for each position in analogBufferTemp
int copyIndex = 0;
float averageVoltage = 0;
float tdsValue = 0;
float temperature = 25;

void setup()
{
  Serial.begin(115200);
  pinMode(TdsSensorPin, INPUT);
}

void loop()
{
  static unsigned long analogSampleTimepoint = millis(); //Starting at millis() = 0
  if (millis() - analogSampleTimepoint > 40U)  //every 40 milliseconds,read the analog value from the ADC 
  {
    analogSampleTimepoint = millis(); //Update analogSampleTimepoint 
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    //read the analog value and store into the buffer (The value (0-1023) is stored in analogBuffer[] at position analogBufferIndex.)
    analogBufferIndex++; //Moving to the next position in the array
    if (analogBufferIndex == SCOUNT) //If buffer is full, reset buffer to prevent overflow
      analogBufferIndex = 0;
  }
  static unsigned long printTimepoint = millis();
  if (millis() - printTimepoint > 800U) //Every 800 milliseconds
  {
    printTimepoint = millis();

    for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++)
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex]; //
    averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
    float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0); //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
    float compensationVolatge = averageVoltage / compensationCoefficient; //temperature compensation
    tdsValue = (133.42 * compensationVolatge * compensationVolatge * compensationVolatge - 255.86 * compensationVolatge * compensationVolatge + 857.39 * compensationVolatge) * 0.5; //convert voltage value to tds value
    //Serial.print("voltage:");
    //Serial.print(averageVoltage,2);
    //Serial.print("V   ");
    Serial.print("TDS----Value:");
    Serial.print(tdsValue, 0);
    Serial.println("ppm");
  }
}
int getMedianNum(int bArray[], int iFilterLen) //Sorts an array of sensor readings and selects the median value from the sorted list (Ignores extreme outliders thus removing noise)
{
   int bTab[iFilterLen];  // Creates a copy of the input array

  for (byte i = 0; i < iFilterLen; i++)
    bTab[i] = bArray[i];  // Copy data to avoid modifying the original

  int i, j, bTemp;

  // Bubble Sort Algorithm: Sorts the array in ascending order
  for (j = 0; j < iFilterLen - 1; j++)
  {
    for (i = 0; i < iFilterLen - j - 1; i++)
    {
      if (bTab[i] > bTab[i + 1])
      {
        // Swap elements if they are in the wrong order
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }

  // Find the median
  if ((iFilterLen & 1) > 0)  // If odd number of elements
    bTemp = bTab[(iFilterLen - 1) / 2]; // Middle value
  else // If even number of elements
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2; // Average of two middle values

  return bTemp; // Return the median value
}