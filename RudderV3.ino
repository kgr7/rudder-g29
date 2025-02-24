#include <Arduino.h>
#include <BleGamepad.h>

// Microcontroller pins. Choose any that has an ADC 
const int GAS_PIN = 36;
const int CLUTCH_PIN = 39;
const int BRAKE_PIN = 34;

// Sampling values for smoothing
const int NUMBER_OF_SAMPLES = 5;
const int DELAY_BETWEEN_SAMPLES = 4;
const int DELAY_BETWEEN_HID_REPORTS = 5;

// Hardware thresholds. Test your own pedals and adjust as necessary
const int MAX_POT_READING = 4095;
const int LEFT_LOWER_THRESHOLD = 200;
const int LEFT_UPPER_THRESHOLD = 3500;
const int RIGHT_LOWER_THRESHOLD = 100;
const int RIGHT_UPPER_THRESHOLD = 3600;
const int BRAKE_LOWER_THRESHOLD = 200;
const int BRAKE_UPPER_THRESHOLD = 2000; // brake pedal is really hard to press all the way

BleGamepad bleGamepad("ESP32 Rudder Pedal", "kgr", 100);

void setup()
{
    Serial.begin(115200);
    Serial.println("Starting BLE work!");
    BleGamepadConfiguration bleGamepadConfig;
    bleGamepadConfig.setIncludeSlider1(true);
    bleGamepadConfig.setIncludeSlider2(true);
    bleGamepad.begin(&bleGamepadConfig);
}

void loop()
{
    if (bleGamepad.isConnected())
    {
      int leftValues[NUMBER_OF_SAMPLES];
      int rightValues[NUMBER_OF_SAMPLES];
      int brakeValues[NUMBER_OF_SAMPLES];

      int leftValue = 0;
      int rightValue = 0;
      int brakeValue = 0;

      for (int i = 0; i < NUMBER_OF_SAMPLES; i++)
      {
          leftValues[i] = analogRead(CLUTCH_PIN);
          rightValues[i] = analogRead(GAS_PIN);
          brakeValues[i] = analogRead(BRAKE_PIN);

          leftValue += leftValues[i];
          rightValue += rightValues[i];
          brakeValue += brakeValues[i];

          delay(DELAY_BETWEEN_SAMPLES);
      }

      leftValue /= NUMBER_OF_SAMPLES;
      rightValue /= NUMBER_OF_SAMPLES;
      brakeValue /= NUMBER_OF_SAMPLES;

      brakeValue = MAX_POT_READING - brakeValue; // scale is reversed on brake pedal

      // adjust for skewed readings from each pedal
      leftValue = adjustPedalThresholds(leftValue, LEFT_LOWER_THRESHOLD, LEFT_UPPER_THRESHOLD);
      rightValue = adjustPedalThresholds(rightValue, RIGHT_LOWER_THRESHOLD, RIGHT_UPPER_THRESHOLD);
      brakeValue = adjustPedalThresholds(brakeValue, BRAKE_LOWER_THRESHOLD, BRAKE_UPPER_THRESHOLD);

      int adjustedLeft = (MAX_POT_READING - leftValue) * 8;
      int adjustedRight = (MAX_POT_READING - rightValue) * 8;
      int adjustedBrake = brakeValue * 8;

      debugPedals(leftValue, rightValue, brakeValue);
      int netRudder = calculateRudderPosition(adjustedLeft, adjustedRight);

      debugRudder(netRudder);

      bleGamepad.setSlider1(netRudder);
      bleGamepad.setSlider2(adjustedBrake);
                
      bleGamepad.sendReport();
      delay(DELAY_BETWEEN_HID_REPORTS);
    }     
}

int adjustPedalThresholds(int pedal, int minThreshold, int maxThreshold) {
    if (pedal > maxThreshold) {
        pedal = MAX_POT_READING;
    } else if (pedal < minThreshold) {
        pedal = 0;
    }
    return pedal;
}

int calculateRudderPosition(int left, int right) {
  int netRudder = 16383;

  if (left < 750) {
    left = 0;
  }

  if (right < 750) {
    right = 0;
  }

  if (left > right) {
    netRudder -= left;
  }
  else if (left < right){
    netRudder += right;
  }

  if (netRudder < 1000) {
    netRudder = 0;
  }

  if (netRudder > 31000) {
    netRudder = 16383 * 2;
  }

  return netRudder;
}

void debugRudder(int rudder) {
  Serial.print("RUDDER:");
  Serial.println(rudder);
}

void debugPedals(int left, int right, int brake) {
    Serial.print("LEFT:");
    Serial.print(left);
    Serial.print(",");
    Serial.print("RIGHT:");
    Serial.print(right);
    Serial.print(",");
    Serial.print("BRAKE:");
    Serial.println(brake);
}
