# Audience Reactive Eye Sculpture

This Arduino-based project is an interactive LED sculpture that reacts to sound in its environment. Using a MAX7219 8x8 LED matrix and a noise sensor, the sculpture displays a blinking eye that crosses its pupils in response to loud sounds from the audience.

## Features

- 👁️ Animated eye that blinks and changes pupil direction.
- 🎤 Sound-reactive behavior using a noise sensor.
- 💥 Crossed-eye animation triggered by loud noises.
- 🎲 Randomized pupil direction for lifelike motion.

## Hardware Used

- **Arduino (Uno or compatible)**
- **MAX7219 8x8 LED Matrix**
- **Analog Sound Sensor Module** (e.g. KY-038 or similar)
- Jumper wires and breadboard

## Wiring Overview

| Component        | Arduino Pin |
|------------------|-------------|
| MAX7219 DIN      | 11          |
| MAX7219 CLK      | 9           |
| MAX7219 CS       | 10          |
| Sound Sensor OUT | A0          |
| Random seed pin  | A1 (unconnected) |

## Installation

1. **Install the `LedControl` library**  
   You can install it via the Arduino IDE Library Manager (`Sketch > Include Library > Manage Libraries...`) by searching for **LedControl**.

2. **Upload the Code**  
   Open the provided `.ino` file in the Arduino IDE and upload it to your board.

3. **Connect the Hardware**  
   Follow the wiring table above to connect the MAX7219 and sound sensor to your Arduino.

## Behavior

- The LED matrix continuously displays an animated blinking eye.
- Loud sounds (like clapping or speaking near the sensor) trigger a "cross-eyed" animation.
- The system uses basic noise filtering by requiring multiple consistent sound hits to avoid false triggers.
