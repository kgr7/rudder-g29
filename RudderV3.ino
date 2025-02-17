/*
 * This example turns the ESP32 into a Bluetooth LE gamepad that presses buttons and moves axis
 *
 * Possible buttons are:
 * BUTTON_1 through to BUTTON_128 (Windows gamepad tester only visualises the first 32)
 ^ Use http://www.planetpointy.co.uk/joystick-test-application/ to visualise all of them
 * Whenever you adjust the amount of buttons/axes etc, make sure you unpair and repair the BLE device
 *
 * Possible DPAD/HAT switch position values are:
 * DPAD_CENTERED, DPAD_UP, DPAD_UP_RIGHT, DPAD_RIGHT, DPAD_DOWN_RIGHT, DPAD_DOWN, DPAD_DOWN_LEFT, DPAD_LEFT, DPAD_UP_LEFT
 *
 * bleGamepad.setAxes takes the following int16_t parameters for the Left/Right Thumb X/Y, Left/Right Triggers plus slider1 and slider2:
 * (Left Thumb X, Left Thumb Y, Right Thumb X, Right Thumb Y, Left Trigger, Right Trigger, Slider 1, Slider 2) (x, y, z, rx, ry, rz)
 *
 * bleGamepad.setHIDAxes instead takes them in a slightly different order (x, y, z, rz, rx, ry)
 *
 * bleGamepad.setLeftThumb (or setRightThumb) takes 2 int16_t parameters for x and y axes (or z and rZ axes)
 * bleGamepad.setRightThumbAndroid takes 2 int16_t parameters for z and rx axes
 *
 * bleGamepad.setLeftTrigger (or setRightTrigger) takes 1 int16_t parameter for rX axis (or rY axis)
 *
 * bleGamepad.setSlider1 (or setSlider2) takes 1 int16_t parameter for slider 1 (or slider 2)
 *
 * bleGamepad.setHat1 takes a hat position as above (or 0 = centered and 1~8 are the 8 possible directions)
 *
 * setHats, setTriggers and setSliders functions are also available for setting all hats/triggers/sliders at once
 *
 * The example shows that you can set axes/hats independantly, or together.
 *
 * It also shows that you can disable the autoReport feature (enabled by default), and manually call the sendReport() function when wanted
 *
 */

#include <Arduino.h>
#include <BleGamepad.h>

const int rightPin = 36;
const int leftPin = 39;

const int numberOfSamples = 5;
const int delayBetweenSamples = 4;
const int delayBetweenHidReports = 5;



BleGamepad bleGamepad("ESP32 Rudder Pedal", "kgr", 100);

void setup()
{
    Serial.begin(115200);
    Serial.println("Starting BLE work!");
    BleGamepadConfiguration bleGamepadConfig;
    bleGamepadConfig.setIncludeRudder(true);
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

      int leftValues[numberOfSamples];
      int rightValues[numberOfSamples];
      
      int leftValue = 0;
      int rightValue = 0;

      for (int i = 0; i < numberOfSamples; i++)
      {
          leftValues[i] = analogRead(leftPin);
          rightValues[i] = analogRead(rightPin);
          leftValue += leftValues[i];
          rightValue += rightValues[i];

          delay(delayBetweenSamples);
      }

      leftValue = leftValue / numberOfSamples;
      rightValue = rightValue / numberOfSamples;

      int adjustedLeft = map(leftValue, 3700, 0, 0, 16383);
      int adjustedRight = map(rightValue, 3700, 0, 0, 16383);

      int netRudder = 16383;

      if (adjustedLeft < 750) {
        adjustedLeft = 0;
      }

      if (adjustedRight < 750) {
        adjustedRight = 0;
      }

      if (adjustedLeft > adjustedRight) {
        netRudder -= adjustedLeft;
      }
      else if (adjustedLeft < adjustedRight){
        netRudder += adjustedRight;
      }

      if (netRudder < 1000) {
        netRudder = 0;
      }

      if (netRudder > 31000) {
        netRudder = 16383 * 2;
      }

      bleGamepad.setSlider1(netRudder);
      Serial.print("NET RUDDER: ");
      Serial.println(netRudder);
                
      bleGamepad.sendReport();
      delay(delayBetweenHidReports);
    }     
}

int calculateRudderPosition(int left, int right) {
  int netRudder = 0;
  
}
