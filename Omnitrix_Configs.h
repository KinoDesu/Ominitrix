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
     _______________________        _________________________
    | **DISPLAY** | **PIN** |      |  **ENCODER**  | **PIN** |
    |:-----------:|:-------:|      |:-------------:|:-------:|
    |     VCC     |  3.3V   |      |      +        |  3.3V   |
    |     GND     |  GROUND |      |     GND       |  GROUND |
    |     SCL     |  35     |      |     SW        |  7      |
    |     SDA     |  33     |      |     DT        |  40     |
    |     DC      |  2      |      |     CLK       |  39     |
    |     CS      |  10     |      |_______________|_________|
    |     RST     |  4      |
    |_____________|_________|
*/

#include <PNGdec.h>
#include <TFT_eSPI.h>
#include "SPI.h"
#include "Omnitrix_Aliens.h"
#include "Omnitrix_Alien_Backround.h"
#include "Omnitrix_Animation.h"

#define buzzer 12
#define potentiometer 13
#define btnAlienChooser 7
#define btnActivate 17

#define ROTARY_PINCLK 39
#define ROTARY_PINDT 40

#define DISPLAY_WIDTH 240
#define DISPLAY_HEIGHT 240
#define IMAGE_WIDTH 100
#define IMAGE_HEIGHT 120
#define MAX_IMAGE_WIDTH 240

#define OMNITRIX_GREEN 0x12319721
#define OMNITRIX_WHITE 0xFFFF
#define OMNITRIX_RED 0xF000

TFT_eSPI tft = TFT_eSPI();
PNG png;

int16_t xpos = 0;
int16_t ypos = 0;

boolean isAlienForm = false;
boolean isActivate = false;
boolean isSameRecharge = false;
boolean hasLastAlien = false;

long elapsedTime = 0L;
long morphedTime = 0L;
long minMorphTime = 5000L;
long deschargeTime = 10000L;
long batteryValue = deschargeTime;
long loopStart;

int alienNo = 0;
int lastAlienValue = 0;

volatile bool rotaryEncoder = false;

void IRAM_ATTR rotary()
{
    rotaryEncoder = true;
}

int8_t checkRotaryEncoder()
{
    // Reset the flag that brought us here (from ISR)
    rotaryEncoder = false;

    static uint8_t lrmem = 3;
    static int lrsum = 0;
    static int8_t TRANS[] = {0, -1, 1, 14, 1, 0, 14, -1, -1, 14, 0, 1, 14, 1, -1, 0};

    // Read BOTH pin states to deterimine validity of rotation (ie not just switch bounce)
    int8_t l = digitalRead(ROTARY_PINCLK);
    int8_t r = digitalRead(ROTARY_PINDT);

    // Move previous value 2 bits to the left and add in our new values
    lrmem = ((lrmem & 0x03) << 2) + 2 * l + r;

    // Convert the bit pattern to a movement indicator (14 = impossible, ie switch bounce)
    lrsum += TRANS[lrmem];

    /* encoder not in the neutral (detent) state */
    if (lrsum % 4 != 0)
    {
        return 0;
    }

    /* encoder in the neutral state - clockwise rotation*/
    if (lrsum == 4)
    {
        lrsum = 0;
        return 1;
    }

    /* encoder in the neutral state - anti-clockwise rotation*/
    if (lrsum == -4)
    {
        lrsum = 0;
        return -1;
    }

    // An impossible rotation has been detected - ignore the movement
    lrsum = 0;
    return 0;
}