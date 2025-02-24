# Use Logitech G29 as a rudder pedal in flight sims

## Requirements
 - ESP32 microcontroller
 - Some wires
 - breadboard
 - DB9 breakout connector (analog pedal plug)
 - Bluetooth equipped Windows PC
   - Other OS's untested, assuming it doesn't work
   - Newer ESP32 boards might be able to work directly with USB. This code uses bluetooth since that's all my board supports.

## Wiring
 - DB9 Pin 1 -> GND
 - DB9 Pin 2 -> 36
 - DB9 Pin 3 -> 34
 - DB9 Pin 4 -> 39
 - DB9 Pin 6 -> 3v3
 - DB9 Pin 9 -> 3v3 (unsure if both pins needed, i guess not)
