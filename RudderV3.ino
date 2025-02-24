#include <Arduino.h>
#include <BleGamepad.h>

const int rightPin = 36;
const int leftPin = 39;
const int brakePin = 34;

const int numberOfSamples = 5;
const int delayBetweenSamples = 4;
const int delayBetweenHidReports = 5;

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
      int leftValues[numberOfSamples];
      int rightValues[numberOfSamples];
      int brakeValues[numberOfSamples];

      int leftValue = 0;
      int rightValue = 0;
      int brakeValue = 0;

      for (int i = 0; i < numberOfSamples; i++)
      {
          leftValues[i] = analogRead(leftPin);
          rightValues[i] = analogRead(rightPin);
          brakeValues[i] = analogRead(brakePin);

          leftValue += leftValues[i];
          rightValue += rightValues[i];
          brakeValue += brakeValues[i];

          delay(delayBetweenSamples);
      }

      leftValue /= numberOfSamples;
      rightValue /= numberOfSamples;
      brakeValue /= numberOfSamples;

      brakeValue = 4095 - brakeValue; // scale is reversed on brake pedal

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
      delay(delayBetweenHidReports);
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
