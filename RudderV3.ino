#include <Arduino.h>
#include <BleGamepad.h>

const int rightPin = 36;
const int leftPin = 39;
const int brakePin = 34;

const int numberOfSamples = 5;
const int delayBetweenSamples = 4;
const int delayBetweenHidReports = 5;

BleGamepad bleGamepad("ESP32 Rudder Pedal", "kgr", 100);

void setup()
{
    Serial.begin(115200);
    Serial.println("Starting BLE work!");
    BleGamepadConfiguration bleGamepadConfig;
    bleGamepadConfig.setIncludeSlider1(true);
    bleGamepadConfig.setIncludeSlider2(true);
    bleGamepad.begin(&bleGamepadConfig); // Creates a gamepad with 128 buttons, 2 hat switches and x, y, z, rZ, rX, rY and 2 sliders (no simulation controls enabled by default)
}

void loop()
{
    if (bleGamepad.isConnected())
    {
      int leftRaw = analogRead(leftPin);
      int rightRaw = analogRead(rightPin);
      int brakeRaw = analogRead(brakePin);

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

      leftValue = leftValue / numberOfSamples;
      rightValue = rightValue / numberOfSamples;
      brakeValue = brakeValue / numberOfSamples;

      int adjustedLeft = map(leftValue, 3700, 0, 0, 16383);
      int adjustedRight = map(rightValue, 3700, 0, 0, 16383);
      int adjustedBrake = map(brakeValue, 4095, 1700, 0, 32767);

      if (adjustedBrake < 2000) {
        adjustedBrake = 0;
      }

      int netRudder = calculateRudderPosition(adjustedLeft, adjustedRight);

      bleGamepad.setSlider1(netRudder);
      bleGamepad.setSlider2(adjustedBrake);

      Serial.print("raw BRAKE: ");
      Serial.println(brakeRaw);

      Serial.print("adjusted BRAKE: ");
      Serial.println(adjustedBrake);
                
      bleGamepad.sendReport();
      delay(delayBetweenHidReports);
    }     
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
