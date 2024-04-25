#include <PNGdec.h>
#include <TFT_eSPI.h>
#include "SPI.h"
#include "omnitrix_aliens.h"
#include "omnitrix_alien_backround.h"
#include "omnitrixAnimation.h"

/*
    Arduino -> UNO
    display -> GC9A01 VER1.0
    _______________________
    | **DISPLAY** | **PIN** |
    |:-----------:|:-------:|
    |     VCC     |  3.3V   |
    |     GND     |  GROUND |
    |     SCL     |  13     |
    |     SDA     |  12     |
    |     DC      |  11     |
    |     CS      |  9      |
    |     RST     |  10     |
    |_____________|_________|
*/

/*
    ESP32 -> S2 mini
    display -> GC9A01 VER1.0
    _______________________
    | **DISPLAY** | **PIN** |
    |:-----------:|:-------:|
    |     VCC     |  3.3V   |
    |     GND     |  GROUND |
    |     SCL     |  18     |
    |     SDA     |  13     |
    |     DC      |  2      |
    |     CS      |  9      |
    |     RST     |  4      |
    |_____________|_________|
*/

#define buzzer 12
#define potentiometer 13
#define btnAlienChooser 7
#define btnActivate 17


#define ROTARY_PINCLK 39
#define ROTARY_PINDT 40

volatile bool rotaryEncoder = false;

void IRAM_ATTR rotary() {
  rotaryEncoder = true;
}

TFT_eSPI tft = TFT_eSPI();
int16_t xpos = 0;
int16_t ypos = 0;

PNG png;

#define DISPLAY_WIDTH 240
#define DISPLAY_HEIGHT 240
#define IMAGE_WIDTH 100
#define IMAGE_HEIGHT 120
#define MAX_IMAGE_WIDTH 240
#define GIF_IMAGE omniStartGIF

boolean isAlienForm = false;
boolean isActivate = false;
boolean isSameRecharge = false;
boolean hasLastAlien = false;
int lastAlienValue = 0;
long elapsedTime = 0L;
long morphedTime = 0L;
long minMorphTime = 5000L;
long deschargeTime = 15000L;
long batteryValue = deschargeTime;
long loopStart;
int alienNo = 0;

void setup() {
  Serial.begin(115200);

  tft.begin();
  tone(buzzer, 800);
  delay(100);
  noTone(buzzer);
  delay(10);
  tone(buzzer, 1600);
  delay(100);
  noTone(buzzer);

  pinMode(potentiometer, INPUT);
  pinMode(btnAlienChooser, INPUT_PULLUP);
  pinMode(btnActivate, INPUT);
  pinMode(buzzer, OUTPUT);

  pinMode(ROTARY_PINCLK, INPUT);
  pinMode(ROTARY_PINDT, INPUT);

  attachInterrupt(digitalPinToInterrupt(ROTARY_PINCLK), rotary, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ROTARY_PINDT), rotary, CHANGE);

  tft.fillScreen(0x12319721);
  int16_t rc = png.openFLASH((uint8_t *)omnitrix_anim[0], sizeof(omnitrix_anim[0]), pngDraw);

  if (rc == PNG_SUCCESS) {
    tft.startWrite();
    rc = png.decode(NULL, 0);
    tft.endWrite();
    tft.endWrite();

    // png.close(); // Required for files, not needed for FLASH arrays
  }
}

void loop() {
  loopStart = millis();

  if (digitalRead(btnActivate) == HIGH) {
    // travado enquanto o botão estiver segurado
    while (digitalRead(btnActivate) == HIGH)
      ;
    omnitrixModeSet();
  }

  omnitrixRecharge();
}

void omnitrixModeSet() {
  // se desligado e com bateria maior q 0
  if (isActivate == false && batteryValue > minMorphTime) {
    // liga o omnitrix
    isActivate = true;

    omnitrixStartup();
    alienSelect();
  }
  // se desligado e sem bateria
  else if (isActivate == false && batteryValue < minMorphTime) {
    // toca o som
    tone(buzzer, 125);
    delay(250);
    noTone(buzzer);
    delay(100);

    // se ligado
  } else if (isActivate == true) {
    // desliga o omnitrix
    isActivate = false;
    omnitrixShutdown();
    delay(100);
  }
}

void omnitrixStartup() {

  int16_t rc = png.openFLASH((uint8_t *)omnitrix_anim[0], sizeof(omnitrix_anim[0]), pngDraw);

  if (rc == PNG_SUCCESS) {
    tft.startWrite();
    rc = png.decode(NULL, 0);
    tft.endWrite();
    tft.endWrite();

    // png.close(); // Required for files, not needed for FLASH arrays
  }
  // toca o som
  tone(buzzer, 1000);
  delay(100);
  noTone(buzzer);
  delay(10);
  tone(buzzer, 2000);
  delay(100);
  noTone(buzzer);
  delay(100);
}

void getAlienNo() {
  if (rotaryEncoder) {
    // Get the movement (if valid)
    int8_t rotationValue = checkRotaryEncoder();

    // If valid movement, do something
    if (rotationValue != 0) {
      alienNo += rotationValue;
      if (alienNo < 0) {
        alienNo = alienCount - 1;
      }
      if (alienNo >= alienCount) {
        alienNo = 0;
      }
    }
  }
}

void alienSelect() {
  tft.fillScreen(0x12319721);

  int frame = 0;
  int fps = 10;

  for (; frame <= omnitrix_anim_N; frame++) {

    int16_t rc = png.openFLASH((uint8_t *)omnitrix_anim[frame], sizeof(omnitrix_anim[frame]), pngDraw);

    if (rc == PNG_SUCCESS) {
      tft.startWrite();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      tft.endWrite();

      // png.close(); // Required for files, not needed for FLASH arrays
    }
    delay(fps);
  }
  delay(100);

  changeAlien(lastAlienValue);

  while (isActivate == true) {

    if (digitalRead(btnActivate) == HIGH) {
      while (digitalRead(btnActivate) == HIGH)
        ;
      isActivate = false;
      omnitrixShutdown();
      return;
    }

    getAlienNo();

    if (hasLastAlien == false) {
      changeAlien(alienNo);
      hasLastAlien = true;
      lastAlienValue = alienNo;
    }

    if (alienNo != lastAlienValue) {
      lastAlienValue = alienNo;
      tone(buzzer, 125);
      delay(100);
      noTone(buzzer);
      changeAlien(alienNo);
    }



    if (digitalRead(btnAlienChooser) == LOW) {
      while (digitalRead(btnAlienChooser) == LOW)
        ;

      hasLastAlien = false;
      isActivate = false;
      isAlienForm = true;
      isSameRecharge = false;
      tone(buzzer, 367);
      delay(100);
      noTone(buzzer);
      fadeInScreen(0xFFFF);

      long morphStart = millis();
      while (isAlienForm) {
        morphedTime = millis() - morphStart;
        if (morphedTime >= batteryValue) {
          batteryValue -= morphedTime;

          if (batteryValue < 0) {
            batteryValue = 0;
          }

          omnitrixDescharge();
          isAlienForm = false;
          loopStart = millis();
        }

        if (digitalRead(btnActivate) == HIGH && morphedTime >= minMorphTime) {
          while (digitalRead(btnActivate) == HIGH)
            ;
          batteryValue -= morphedTime;

          if (batteryValue < 0) {
            batteryValue = 0;
          }

          omnitrixShutdown();
          isAlienForm = false;
          loopStart = millis();
        }
      }
    }
  }
}

void changeAlien(int alienNo) {

  int16_t rc = png.openFLASH((uint8_t *)omnitrix_alien_backround, sizeof(omnitrix_alien_backround), pngDraw);

  if (rc == PNG_SUCCESS) {
    tft.startWrite();
    rc = png.decode(NULL, 0);
    tft.endWrite();
    tft.endWrite();

    // png.close(); // Required for files, not needed for FLASH arrays
  }

  delay(100);

  rc = png.openFLASH((uint8_t *)omnitrix_aliens[alienNo], sizeof(omnitrix_aliens[alienNo]), pngDraw);

  if (rc == PNG_SUCCESS) {
    tft.startWrite();
    rc = png.decode(NULL, 0);
    tft.endWrite();
    tft.endWrite();

    // png.close(); // Required for files, not needed for FLASH arrays
  }
}

void omnitrixShutdown() {

  // toca o som
  tone(buzzer, 250);
  delay(100);
  noTone(buzzer);
  tft.fillScreen(0x12319721);
  int16_t rc = png.openFLASH((uint8_t *)omnitrix_anim[0], sizeof(omnitrix_anim[0]), pngDraw);

  if (rc == PNG_SUCCESS) {
    tft.startWrite();
    rc = png.decode(NULL, 0);
    tft.endWrite();
    tft.endWrite();

    // png.close(); // Required for files, not needed for FLASH arrays
  }
}

void omnitrixDescharge() {
  isAlienForm = false;

  // vermelho
  fadeInScreen(0xF000);
  // toca
  tone(buzzer, 125);
  delay(250);
  noTone(buzzer);
  // branco
  fadeInScreen(0xFFFF);

  // vermelho
  fadeInScreen(0xF000);
  // toca
  tone(buzzer, 125);
  delay(250);
  noTone(buzzer);
  // branco
  fadeInScreen(0xFFFF);

  // vermelho
  fadeInScreen(0xF000);
  tone(buzzer, 125);
  delay(500);
  noTone(buzzer);
  fadeInScreen(0xF000);
}

void omnitrixRecharge() {
  if (batteryValue < deschargeTime) {
    delay(100);
    elapsedTime += millis() - loopStart;
    batteryValue += elapsedTime;

    if (batteryValue >= minMorphTime && isSameRecharge == false) {
      morphedTime = 0L;
      tone(buzzer, 800);
      delay(100);
      noTone(buzzer);
      delay(10);
      tone(buzzer, 1600);
      delay(100);
      noTone(buzzer);
      isSameRecharge = true;
      fadeInScreen(0x0F00);
    }

    if (batteryValue >= deschargeTime) {
      batteryValue = deschargeTime;
      tone(buzzer, 800);
      delay(100);
      noTone(buzzer);
      delay(10);
      tone(buzzer, 1600);
      delay(100);
      noTone(buzzer);
      tone(buzzer, 1600);
      delay(500);
      noTone(buzzer);
      isSameRecharge = false;
    }

    elapsedTime = 0L;
  }
}

void fadeInScreen(uint16_t color) {
  tft.fillCircle(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2, DISPLAY_WIDTH / 2 + 1, color);
  int16_t rc = png.openFLASH((uint8_t *)omnitrix_anim[0], sizeof(omnitrix_anim[0]), pngDraw);

  if (rc == PNG_SUCCESS) {
    tft.startWrite();
    rc = png.decode(NULL, 0);
    tft.endWrite();
    tft.endWrite();

    // png.close(); // Required for files, not needed for FLASH arrays
  }
}

int8_t checkRotaryEncoder() {
  // Reset the flag that brought us here (from ISR)
  rotaryEncoder = false;

  static uint8_t lrmem = 3;
  static int lrsum = 0;
  static int8_t TRANS[] = { 0, -1, 1, 14, 1, 0, 14, -1, -1, 14, 0, 1, 14, 1, -1, 0 };

  // Read BOTH pin states to deterimine validity of rotation (ie not just switch bounce)
  int8_t l = digitalRead(ROTARY_PINCLK);
  int8_t r = digitalRead(ROTARY_PINDT);

  // Move previous value 2 bits to the left and add in our new values
  lrmem = ((lrmem & 0x03) << 2) + 2 * l + r;

  // Convert the bit pattern to a movement indicator (14 = impossible, ie switch bounce)
  lrsum += TRANS[lrmem];

  /* encoder not in the neutral (detent) state */
  if (lrsum % 4 != 0) {
    return 0;
  }

  /* encoder in the neutral state - clockwise rotation*/
  if (lrsum == 4) {
    lrsum = 0;
    return 1;
  }

  /* encoder in the neutral state - anti-clockwise rotation*/
  if (lrsum == -4) {
    lrsum = 0;
    return -1;
  }

  // An impossible rotation has been detected - ignore the movement
  lrsum = 0;
  return 0;
}