#include "LedControl.h"

LedControl lc = LedControl(11, 9, 10, 1); // DATA, CLK, CS, Number of displays

// Timing variables
unsigned long lastBlinkTime = 0;
unsigned long lastEyeUpdateTime = 0;
unsigned long lastSensorReadTime = 0;
unsigned long eyeStateTime = 0;
const unsigned long BLINK_INTERVAL = 3000;      // Time between blinks (ms)
const unsigned long SENSOR_READ_INTERVAL = 50;  // How often to read the sensor (ms)
const unsigned long EYE_UPDATE_INTERVAL = 100;  // How often to update eye animation (ms)

// Cross Eye frame
byte crossEyes[8] = {
  B10000001, B01000010, B00100100, B00011000, 
  B00011000, B00100100, B01000010, B10000001
};

// Eye frames for blinking, pupil center
byte eyeFramesC[][8] = {
  { B00111100, B01111110, B11111111, B11100111, B11100111, B11111111, B01111110, B00111100 }, // Open with pupil
  { B00000000, B01111110, B11111111, B11100111, B11100111, B11111111, B01111110, B00111100 }, // Half-blink
  { B00000000, B00000000, B00111100, B11111111, B11111111, B11111111, B00111100, B00000000 }, // Almost closed
  { B00000000, B00000000, B00000000, B00111100, B11111111, B01111110, B00011000, B00000000 }, // Almost closed 2
  { B00000000, B00000000, B00000000, B00000000, B00000000, B01111110, B00000000, B00000000 }  // Fully closed
};

// Eye frames for blinking, pupil left and right (unchanged from previous code)
byte eyeFramesL[][8] = {
  { B00111100, B01111110, B11111111, B10011111, B10011111, B11111111, B01111110, B00111100 },
  { B00000000, B01111110, B11111111, B10011111, B10011111, B11111111, B01111110, B00111100 },
  { B00000000, B00000000, B00111100, B11111111, B11111111, B11111111, B00111100, B00000000 },
  { B00000000, B00000000, B00000000, B00111100, B11111111, B01111110, B00011000, B00000000 },
  { B00000000, B00000000, B00000000, B00000000, B00000000, B01111110, B00000000, B00000000 }
};

byte eyeFramesR[][8] = {
  { B00111100, B01111110, B11111111, B11111001, B11111001, B11111111, B01111110, B00111100 },
  { B00000000, B01111110, B11111111, B11111001, B11111001, B11111111, B01111110, B00111100 },
  { B00000000, B00000000, B00111100, B11111111, B11111111, B11111111, B00111100, B00000000 },
  { B00000000, B00000000, B00000000, B00111100, B11111111, B01111110, B00011000, B00000000 },
  { B00000000, B00000000, B00000000, B00000000, B00000000, B01111110, B00000000, B00000000 }
};

// State variables
enum EyeState {NORMAL, BLINKING, CROSSED};
EyeState currentEyeState = NORMAL;
int blinkFrame = 0;
int pupilPosition = 0; // 0=center, 1=left, 2=right

// Digital sound sensor variables
const int SOUND_SENSOR_PIN = A0; 
const unsigned long CROSS_EYE_DURATION = 1000;  // How long to show crossed eyes (ms)
const unsigned long NOISE_COOLDOWN = 2000;      // Cooldown period after noise trigger (ms)
unsigned long lastNoiseTriggerTime = 0;
bool noiseTriggered = false;

// For digital sound detection
const int NOISE_DETECT_SAMPLES = 10;
int noiseHitCount = 0;

void setup() {
  Serial.begin(9600);
  
  // Initialize LED matrix
  lc.shutdown(0, false);
  lc.setIntensity(0, 8);
  lc.clearDisplay(0);
  
  // Initialize random seed
  randomSeed(analogRead(A1)); // Use an unconnected analog pin for random seed
  
  // Set initial pupil position randomly
  updatePupilPosition();
  Serial.println("Digital sound sensor mode");
  Serial.println("Make loud noises to test");
}

void updatePupilPosition() {
  int randValue = random(1, 30);
  if (randValue < 20) {
    pupilPosition = 0; // Center
  } else if (randValue < 25) {
    pupilPosition = 1; // Left
  } else {
    pupilPosition = 2; // Right
  }
}

void drawEyeFrame(int frameIndex) {
  // Draw based on current pupil position 
  switch (pupilPosition) {
    case 0: // Center
      for (int i = 0; i < 8; i++) {
        lc.setRow(0, i, eyeFramesC[frameIndex][i]);
      }
      break;
    case 1: // Left
      for (int i = 0; i < 8; i++) {
        lc.setRow(0, i, eyeFramesL[frameIndex][i]);
      }
      break;
    case 2: // Right
      for (int i = 0; i < 8; i++) {
        lc.setRow(0, i, eyeFramesR[frameIndex][i]);
      }
      break;
  }
}

void drawCrossEyes() {
  for (int i = 0; i < 8; i++) {
    lc.setRow(0, i, crossEyes[i]);
  }
}

bool detectSoundDigital() {
  int sensorValue = analogRead(SOUND_SENSOR_PIN);
  bool soundDetected = (sensorValue > 500);  // Adjust this threshold

  // debug info
  Serial.print("Sound:");
  Serial.print(sensorValue);
  Serial.print(" Detected:");
  Serial.println(soundDetected ? "YES" : "no");
  return soundDetected;
}

void updateEyeState() {
  unsigned long currentTime = millis();

  // Only check for sound periodically
  if (currentTime - lastSensorReadTime >= SENSOR_READ_INTERVAL) {
    lastSensorReadTime = currentTime;
    
    // Use simple hit counting for more reliable detection
    if (detectSoundDigital()) {
      noiseHitCount++;
      if (noiseHitCount >= 3) { // Require multiple hits to trigger
        // Only trigger if not in cooldown period
        if (!noiseTriggered && (currentTime - lastNoiseTriggerTime > NOISE_COOLDOWN)) {
          currentEyeState = CROSSED;
          eyeStateTime = currentTime;
          lastNoiseTriggerTime = currentTime;
          noiseTriggered = true;
          Serial.println("*** NOISE DETECTED! CROSSING EYES ***"); //for debug
        }
        noiseHitCount = 0; // Reset counter after trigger
      }
    } else {
      // Gradually reduce the hit count if no sound detected
      if (noiseHitCount > 0 && (currentTime % 200 == 0)) {
        noiseHitCount--;
      }
    }
    // Reset noise trigger flag after cooldown
    if (noiseTriggered && (currentTime - lastNoiseTriggerTime > NOISE_COOLDOWN)) {
      noiseTriggered = false;
    }
  }
  //Handle blinking
  if (currentEyeState == NORMAL && (currentTime - lastBlinkTime >= BLINK_INTERVAL)) {
    currentEyeState = BLINKING;
    blinkFrame = 0;
    eyeStateTime = currentTime;
    lastBlinkTime = currentTime;
    Serial.println("Blinking...");
  }
  
  // Handle state transitions
  switch (currentEyeState) {
    case CROSSED:
      // Exit crossed eye state after duration
      if (currentTime - eyeStateTime >= CROSS_EYE_DURATION) {
        currentEyeState = NORMAL;
        updatePupilPosition(); // Randomize pupil position when returning to normal
        Serial.println("Returning to normal eye state");
      }
      break;
      
    case BLINKING:
      // Update blink animation
      if (currentTime - eyeStateTime >= 100) { // Progress through blink frames every 100ms
        blinkFrame++;
        eyeStateTime = currentTime;
        
        // Complete blink cycle
        if (blinkFrame > 8) { // 4 frames down, 4 frames up
          currentEyeState = NORMAL;
          updatePupilPosition(); // Randomize pupil position when returning to normal
          Serial.println("Blink complete");
        }
      }
      break;
      
    case NORMAL:
      // Randomly update pupil position occasionally when in normal state
      if (random(100) < 2 && currentTime - eyeStateTime >= 500) { // 2% chance every 500ms
        updatePupilPosition();
        eyeStateTime = currentTime;
      }
      break;
  }
}

void updateDisplay() {
  switch (currentEyeState) {
    case NORMAL:
      drawEyeFrame(0); // Open eye
      break;
      
    case BLINKING:
      // First half of blink goes down, second half goes up
      if (blinkFrame <= 4) {
        drawEyeFrame(blinkFrame);
      } else {
        drawEyeFrame(8 - blinkFrame); // Going back up
      }
      break;
      
    case CROSSED:
      drawCrossEyes();
      break;
  }
}

void loop() {
  unsigned long currentTime = millis();
  updateEyeState();
  if (currentTime - lastEyeUpdateTime >= EYE_UPDATE_INTERVAL) {
    lastEyeUpdateTime = currentTime;
    updateDisplay();
  }
}
