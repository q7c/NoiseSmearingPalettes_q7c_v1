// Source code is based on https://github.com/adafruit/Adafruit_NeoMatrix
// replace internal use of NeoPixel library with CRGB array to use with FastLED
//
// modified:  Juergen Skrotzky (JorgenVikingGod@gmail.com)
// date:      2016/04/27
// --------------------------------------------------------------------
// Original copyright & description below
// --------------------------------------------------------------------
// Adafruit_NeoMatrix example for single NeoPixel Shield.
// Scrolls 'Howdy' across the matrix in a portrait (vertical) orientation.

#include <FastLED.h>
#include <FastLED_GFX.h>
#include <FastLEDMatrix.h>

#define LED_PIN        2
#define COLOR_ORDER    GRB
#define CHIPSET        WS2812B

#define MATRIX_WIDTH   16 // width of FastLED matrix
#define MATRIX_HEIGHT  16 // height of matrix
#define MATRIX_TYPE    (MTX_MATRIX_BOTTOM + MTX_MATRIX_LEFT + MTX_MATRIX_COLUMNS + MTX_MATRIX_ZIGZAG) // matrix layout flags, add together as needed
#define NUM_LEDS (MATRIX_WIDTH * MATRIX_HEIGHT)
/*
  // MATRIX DECLARATION:
  // Parameter 1 = width of FastLED matrix
  // Parameter 2 = height of matrix
  // Parameter 3 = matrix layout flags, add together as needed:
  //   MTX_MATRIX_TOP, MTX_MATRIX_BOTTOM, MTX_MATRIX_LEFT, MTX_MATRIX_RIGHT:
  //     Position of the FIRST LED in the matrix; pick two, e.g.
  //     MTX_MATRIX_TOP + MTX_MATRIX_LEFT for the top-left corner.
  //   MTX_MATRIX_ROWS, MTX_MATRIX_COLUMNS: LEDs are arranged in horizontal
  //     rows or in vertical columns, respectively; pick one or the other.
  //   MTX_MATRIX_PROGRESSIVE, MTX_MATRIX_ZIGZAG: all rows/columns proceed
  //     in the same order, or alternate lines reverse direction; pick one.
  //   See example below for these values in action.


  // Example for NeoPixel Shield.  In this application we'd like to use it
  // as a 5x8 tall matrix, with the USB port positioned at the top of the
  // Arduino.  When held that way, the first pixel is at the top right, and
  // lines are arranged in columns, progressive order.
*/
#define TARGET_FRAME_TIME   25  // Desired update rate, though if too many leds it will just run as fast as it can!
#define PLASMA_X_FACTOR     24
#define PLASMA_Y_FACTOR     24
#define NUM_LAYERS 1

//CRGB matrix[MATRIX_WIDTH * MATRIX_HEIGHT];
//CRGB matrix2[MATRIX_WIDTH * MATRIX_HEIGHT];

uint32_t x[NUM_LAYERS];
uint32_t y[NUM_LAYERS];
uint32_t z[NUM_LAYERS];
uint32_t scale_x[NUM_LAYERS];
uint32_t scale_y[NUM_LAYERS];

byte CentreX =  (MATRIX_WIDTH / 2) - 1;
byte CentreY = (MATRIX_HEIGHT / 2) - 1;

const bool    kMatrixSerpentineLayout = true;
uint8_t noisesmoothing;
uint8_t noise[NUM_LAYERS][MATRIX_WIDTH][MATRIX_HEIGHT];

cFastLEDSingleMatrix<MATRIX_WIDTH, MATRIX_HEIGHT, MATRIX_TYPE> matrix;
cFastLEDSingleMatrix<MATRIX_WIDTH, MATRIX_HEIGHT, MATRIX_TYPE> matrix2;

uint16_t PlasmaTime, PlasmaShift;
uint32_t LoopDelayMS, LastLoop;

void ESP8266_yield() {
#ifdef ESP8266
  yield(); // secure time for the WiFi stack of ESP8266
#endif
}



void setup() {
  // initial FastLED
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(matrix[0], matrix.Size());
  FastLED.setBrightness(50);
  FastLED.clear(true);


  LoopDelayMS = TARGET_FRAME_TIME;
  LastLoop = millis() - LoopDelayMS;
  PlasmaShift = (random8(0, 5) * 32) + 64;
  PlasmaTime = 0;

    BasicVariablesSetup();
}


int pass = 0;

void RainbowPlasma(){
    if (abs(millis() - LastLoop) >= LoopDelayMS) {
    LastLoop = millis();
    FastLED.clear();
    // Fill background with dim plasma
    for (int16_t x = 0; x < matrix.width(); x++) {
      for (int16_t y = 0; y < matrix.height(); y++) {
        ESP8266_yield(); // secure time for the WiFi stack of ESP8266
        int16_t r = sin16(PlasmaTime) / 256;
        int16_t h = sin16(x * r * PLASMA_X_FACTOR + PlasmaTime) + cos16(y * (-r) * PLASMA_Y_FACTOR + PlasmaTime) + sin16(y * x * (cos16(-PlasmaTime) / 256) / 2);
        matrix(x, y) = CHSV((uint8_t)((h / 256) + 128), 255, 64);
      }
    }
    uint16_t OldPlasmaTime = PlasmaTime;
    PlasmaTime += PlasmaShift;
    if (OldPlasmaTime > PlasmaTime)
      PlasmaShift = (random8(0, 5) * 32) + 64;
/*
  void HorizontalMirror(bool FullHeight = true);
  void VerticalMirror();
  void QuadrantMirror();
  void QuadrantRotateMirror();
  void TriangleTopMirror(bool FullHeight = true);
  void TriangleBottomMirror(bool FullHeight = true);
  void QuadrantTopTriangleMirror();
  void QuadrantBottomTriangleMirror();

 */
  matrix.TriangleBottomMirror(1);

    FastLED.show();
  }
}

void BasicVariablesSetup() {

  noisesmoothing = 200;
  for (int i = 0; i < NUM_LAYERS; i++) {
    x[i] = random16();
    y[i] = random16();
    z[i] = random16();
    scale_x[i] = 6000;
    scale_y[i] = 6000;
  }
}

void MultipleStream() {

  //CLS();
  DimAll(249);

  // gelb im Kreis
  byte xx = 4 + sin8( millis() / 10) / 10;
  byte yy = 4 + cos8( millis() / 10) / 10;
  matrix( xx, yy) = 0xFFFF00;

  // rot in einer Acht
  xx = 8 + sin8( millis() / 46) / 16;
  yy = 8 + cos8( millis() / 15) / 16;
  matrix( xx, yy) = 0xFF0000;

  // Noise
  x[0] += 1000;
  y[0] += 1000;
  scale_x[0] = 4000;
  scale_y[0] = 4000;
  FillNoise(0);

  MoveX(8);
  MoveFractionalNoiseX();

  MoveY(8);
  MoveFractionalNoiseY();
}

void MultipleStream2() {

  DimAll(230);

  byte xx = 4 + sin8( millis() / 9) / 10;
  byte yy = 4 + cos8( millis() / 10) / 10;
  matrix( xx, yy) += 0x0000FF;

  xx = 8 + sin8( millis() / 10) / 16;
  yy = 8 + cos8( millis() / 7) / 16;
  matrix( xx, yy) += 0xFF0000;

  matrix( 15, 15) += 0xFFFF00;

  x[0] += 1000;
  y[0] += 1000;
  z[0] += 1000;
  scale_x[0] = 4000;
  scale_y[0] = 4000;
  FillNoise(0);

  MoveX(3);
  MoveFractionalNoiseY2();

  MoveY(3);
  MoveFractionalNoiseX2();
}

void MultipleStream8() {

  //CLS();
  DimAll(230);

  for (uint8_t y = 1; y < 32; y = y + 6) {
    for (uint8_t x = 1; x < 32; x = x + 6) {

      matrix(x, y) += CHSV((x * y) / 4, 255, 255);
    }
  }



  // Noise
  x[0] += 1000;
  y[0] += 1000;
  z[0] += 1000;
  scale_x[0] = 4000;
  scale_y[0] = 4000;
  FillNoise(0);

  MoveX(3);
  MoveFractionalNoiseX2();

  MoveY(3);
  MoveFractionalNoiseY2();

}

void MultipleStream9() {

  //CLS();
  DimAll(249);

  // yellow circle
  byte xx = 4 + sin8( millis() / 10) / 10;
  byte yy = 4 + cos8( millis() / 10) / 10;
  matrix( xx, yy) = 0xFFFF00;

  // red in a figure eight
  xx = 8 + sin8( millis() / 46) / 16;
  yy = 8 + cos8( millis() / 15) / 16;
  matrix( xx, yy) = 0xFF0000;

  // Noise
  x[0] += 1000;
  y[0] += 1000;
  scale_x[0] = 4000;
  scale_y[0] = 4000;
  FillNoise(0);

  MoveX(8);
  MoveFractionalNoiseX();

  MoveY(8);
  MoveFractionalNoiseY();
}

void MoveFractionalNoiseX(byte amt = 16) {

  // move delta pixelwise
  for (int y = 0; y < MATRIX_HEIGHT; y++) {

    uint16_t amount = noise[0][0][y] * amt;
    byte delta = 31 - (amount / 256);
    byte fractions = amount - (delta * 256);

    for (int x = 0; x < MATRIX_WIDTH - delta; x++) {
      matrix2(x, y) = matrix(x + delta, y);
    }
    for (int x = MATRIX_WIDTH - delta; x < MATRIX_WIDTH; x++) {
      matrix2(x, y) = matrix(x + delta - MATRIX_WIDTH, y);
    }
  }
  //CopyBack();

  //move fractions
  CRGB PixelA;
  CRGB PixelB;

  //byte fract = beatsin8(60);
  //byte y = 1;

  for (uint8_t y = 0; y < MATRIX_HEIGHT; y++) {

    uint16_t amount = noise[0][0][y] * amt;
    byte delta = 31 - (amount / 256);
    byte fractions = amount - (delta * 256);

    for (uint8_t x = 1; x < MATRIX_WIDTH; x++) {
      PixelA = matrix2(x, y);
      PixelB = matrix2(x - 1, y);

      PixelA %= 255 - fractions;
      PixelB %= fractions;


      matrix(x, y) = PixelA + PixelB;
      //matrix2(x, y) = 300;
    }

    PixelA = matrix2(0, y);
    PixelB = matrix2(MATRIX_WIDTH - 1, y);

    PixelA %= 255 - fractions;
    PixelB %= fractions;
    /*
      PixelB.r = dim8_raw(PixelB.r);
      PixelB.g = dim8_raw(PixelB.g);
      PixelB.b = dim8_raw(PixelB.b);
    */


    matrix(0, y) = PixelA + PixelB;

  }

}


void MoveFractionalNoiseY(byte amt = 16) {

  // move delta pixelwise
  for (int x = 0; x < MATRIX_WIDTH; x++) {

    uint16_t amount = noise[0][x][0] * amt;
    byte delta = 31 - (amount / 256);
    byte fractions = amount - (delta * 256);

    for (int y = 0; y < MATRIX_WIDTH - delta; y++) {
      matrix2(x, y) = matrix(x, y + delta);
    }
    for (int y = MATRIX_WIDTH - delta; y < MATRIX_WIDTH; y++) {
      matrix2(x, y) = matrix(x, y + delta - MATRIX_WIDTH);
    }
  }
  //CopyBack();

  //move fractions
  CRGB PixelA;
  CRGB PixelB;

  //byte fract = beatsin8(60);
  //byte y = 1;

  for (uint8_t x = 0; x < MATRIX_HEIGHT; x++) {

    uint16_t amount = noise[0][x][0] * amt;
    byte delta = 31 - (amount / 256);
    byte fractions = amount - (delta * 256);

    for (uint8_t y = 1; y < MATRIX_WIDTH; y++) {
      PixelA = matrix2(x, y);
      PixelB = matrix2(x, y - 1);

      PixelA %= 255 - fractions;
      PixelB %= fractions;


      matrix(x, y) = PixelA + PixelB;
      //matrix2(x, y) = 300;
    }

    PixelA = matrix2(x, 0);
    PixelB = matrix2(x, MATRIX_WIDTH - 1);

    PixelA %= 255 - fractions;
    PixelB %= fractions;
    /*
      PixelB.r = dim8_raw(PixelB.r);
      PixelB.g = dim8_raw(PixelB.g);
      PixelB.b = dim8_raw(PixelB.b);
    */


    matrix(x, 0) = PixelA + PixelB;

  }

}

//MATRIX_WIDTH, MATRIX_HEIGHT
void MoveFractionalNoiseX2() {
  // move delta pixelwise
  for (int y = 0; y < MATRIX_HEIGHT; y++) {

    uint16_t amount = noise[0][0][y] * 4;
    byte delta = 31 - (amount / 256);
    byte fractions = amount - (delta * 256);

    for (int x = 0; x < MATRIX_WIDTH - delta; x++) {
      matrix2(x, y) = matrix(x + delta, y);
    }
    for (int x = MATRIX_WIDTH - delta; x < MATRIX_WIDTH; x++) {
      matrix2(x, y) = matrix(x + delta - MATRIX_WIDTH, y);
    }
  }
  //CopyBack();

  //move fractions
  CRGB PixelA;
  CRGB PixelB;

  //byte fract = beatsin8(60);
  //byte y = 1;

  for (uint8_t y = 0; y < MATRIX_HEIGHT; y++) {

    uint16_t amount = noise[0][0][y] * 4;
    byte delta = 31 - (amount / 256);
    byte fractions = amount - (delta * 256);

    for (uint8_t x = 1; x < MATRIX_WIDTH; x++) {
      PixelA = matrix2(x, y);
      PixelB = matrix2(x - 1, y);

      PixelA %= 255 - fractions;
      PixelB %= fractions;

      matrix(x, y) = PixelA + PixelB;
      //matrix2[XY(x, y)] = 300;
    }

    PixelA = matrix2(0, y);
    PixelB = matrix2(MATRIX_WIDTH - 1, y);

    PixelA %= 255 - fractions;
    PixelB %= fractions;
    /*
      PixelB.r = dim8_raw(PixelB.r);
      PixelB.g = dim8_raw(PixelB.g);
      PixelB.b = dim8_raw(PixelB.b);
    */
    matrix(0, y) = PixelA + PixelB;
  }
}

void MoveFractionalNoiseY2() {

  // move delta pixelwise
  for (int x = 0; x < MATRIX_WIDTH; x++) {

    uint16_t amount = noise[0][x][0] * 4;
    byte delta = 31 - (amount / 256);
    byte fractions = amount - (delta * 256);

    for (int y = 0; y < MATRIX_WIDTH - delta; y++) {
      matrix2(x, y) = matrix(x, y + delta);
    }
    for (int y = MATRIX_WIDTH - delta; y < MATRIX_WIDTH; y++) {
      matrix2(x, y) = matrix(x, y + delta - MATRIX_WIDTH);
    }
  }
  //CopyBack();

  //move fractions
  CRGB PixelA;
  CRGB PixelB;

  //byte fract = beatsin8(60);
  //byte y = 1;

  for (uint8_t x = 0; x < MATRIX_HEIGHT; x++) {

    uint16_t amount = noise[0][x][0] * 4;
    byte delta = 31 - (amount / 256);
    byte fractions = amount - (delta * 256);

    for (uint8_t y = 1; y < MATRIX_WIDTH; y++) {
      PixelA = matrix2(x, y);
      PixelB = matrix2(x, y - 1);

      PixelA %= 255 - fractions;
      PixelB %= fractions;

      matrix(x, y) = PixelA + PixelB;
      //matrix2[XY(x, y)] = 300;
    }

    PixelA = matrix2(x, 0);
    PixelB = matrix2(x, MATRIX_WIDTH - 1);

    PixelA %= 255 - fractions;
    PixelB %= fractions;
    /*
      PixelB.r = dim8_raw(PixelB.r);
      PixelB.g = dim8_raw(PixelB.g);
      PixelB.b = dim8_raw(PixelB.b);
    */
    matrix(x, 0) = PixelA + PixelB;

  }

}

void DimAll(byte value)
{
  for (int i = 0; i < NUM_LEDS; i++) {
    matrix.drawPixel(i, CHSV(value, 255, 64));
  }
}

void MoveX(byte delta) {

  //CLS2();

  for (int y = 0; y < MATRIX_HEIGHT; y++) {

    for (int x = 0; x < MATRIX_WIDTH - delta; x++) {
      matrix2(x, y) = matrix(x + delta, y);
    }
    for (int x = MATRIX_WIDTH - delta; x < MATRIX_WIDTH; x++) {
      matrix2(x, y) = matrix(x + delta - MATRIX_WIDTH, y);
    }
  }


  //CLS();

  // write back to leds
  for (uint8_t y = 0; y < MATRIX_HEIGHT; y++) {
    for (uint8_t x = 0; x < MATRIX_WIDTH; x++) {
      matrix(x, y) = matrix2(x, y);
    }
  }
}

void MoveY(byte delta) {

  //CLS2();

  for (int x = 0; x < MATRIX_WIDTH; x++) {

    for (int y = 0; y < MATRIX_HEIGHT - delta; y++) {
      matrix2(x, y) = matrix(x, y + delta);
    }
    for (int y = MATRIX_HEIGHT - delta; y < MATRIX_HEIGHT; y++) {
      matrix2(x, y) = matrix(x, y + delta - MATRIX_HEIGHT);
    }
  }


  //CLS();

  // write back to leds
  for (uint8_t y = 0; y < MATRIX_HEIGHT; y++) {
    for (uint8_t x = 0; x < MATRIX_WIDTH; x++) {
      matrix(x, y) = matrix2(x, y);
    }
  }
}

void MoveFractionalNoiseX() {

  // move delta pixelwise
  for (int y = 0; y < MATRIX_HEIGHT; y++) {

    uint16_t amount = noise[0][0][y] * 16;
    byte delta = 31 - (amount / 256);
    byte fractions = amount - (delta * 256);

    for (int x = 0; x < MATRIX_WIDTH - delta; x++) {
      matrix2(x, y) = matrix(x + delta, y);
    }
    for (int x = MATRIX_WIDTH - delta; x < MATRIX_WIDTH; x++) {
      matrix2(x, y) = matrix(x + delta - MATRIX_WIDTH, y);
    }
  }
  //CopyBack();

  //move fractions
  CRGB PixelA;
  CRGB PixelB;

  //byte fract = beatsin8(60);
  //byte y = 1;

  for (uint8_t y = 0; y < MATRIX_HEIGHT; y++) {

    uint16_t amount = noise[0][0][y] * 16;
    byte delta = 31 - (amount / 256);
    byte fractions = amount - (delta * 256);

    for (uint8_t x = 1; x < MATRIX_WIDTH; x++) {
      PixelA = matrix2(x, y);
      PixelB = matrix2(x - 1, y);

      PixelA %= 255 - fractions;
      PixelB %= fractions;


      matrix(x, y) = PixelA + PixelB;
      //matrix2(x, y) = 300;
    }

    PixelA = matrix2(0, y);
    PixelB = matrix2(MATRIX_WIDTH - 1, y);

    PixelA %= 255 - fractions;
    PixelB %= fractions;
    /*
      PixelB.r = dim8_raw(PixelB.r);
      PixelB.g = dim8_raw(PixelB.g);
      PixelB.b = dim8_raw(PixelB.b);
    */


    matrix(0, y) = PixelA + PixelB;

  }

}

void MoveFractionalNoiseY() {

  // move delta pixelwise
  for (int x = 0; x < MATRIX_WIDTH; x++) {

    uint16_t amount = noise[0][x][0] * 16;
    byte delta = 31 - (amount / 256);
    byte fractions = amount - (delta * 256);

    for (int y = 0; y < MATRIX_WIDTH - delta; y++) {
      matrix2(x, y) = matrix(x, y + delta);
    }
    for (int y = MATRIX_WIDTH - delta; y < MATRIX_WIDTH; y++) {
      matrix2(x, y) = matrix(x, y + delta - MATRIX_WIDTH);
    }
  }
  //CopyBack();

  //move fractions
  CRGB PixelA;
  CRGB PixelB;

  //byte fract = beatsin8(60);
  //byte y = 1;

  for (uint8_t x = 0; x < MATRIX_HEIGHT; x++) {

    uint16_t amount = noise[0][x][0] * 16;
    byte delta = 31 - (amount / 256);
    byte fractions = amount - (delta * 256);

    for (uint8_t y = 1; y < MATRIX_WIDTH; y++) {
      PixelA = matrix2(x, y);
      PixelB = matrix2(x, y - 1);

      PixelA %= 255 - fractions;
      PixelB %= fractions;


      matrix(x, y) = PixelA + PixelB;
      //matrix2(x, y) = 300;
    }

    PixelA = matrix2(x, 0);
    PixelB = matrix2(x, MATRIX_WIDTH - 1);

    PixelA %= 255 - fractions;
    PixelB %= fractions;
    /*
      PixelB.r = dim8_raw(PixelB.r);
      PixelB.g = dim8_raw(PixelB.g);
      PixelB.b = dim8_raw(PixelB.b);
    */


    matrix(x, 0) = PixelA + PixelB;

  }

}

void FillNoise(byte layer) {

  for (uint8_t i = 0; i < MATRIX_WIDTH; i++) {

    uint32_t ioffset = scale_x[layer] * (i - CentreX);

    for (uint8_t j = 0; j < MATRIX_HEIGHT; j++) {

      uint32_t joffset = scale_y[layer] * (j - CentreY);

      byte data = inoise16(x[layer] + ioffset, y[layer] + joffset, z[layer]) >> 8;

      uint8_t olddata = noise[layer][i][j];
      uint8_t newdata = scale8( olddata, noisesmoothing ) + scale8( data, 256 - noisesmoothing );
      data = newdata;


      noise[layer][i][j] = data;
    }
  }
}

uint16_t XY( uint8_t x, uint8_t y)
{
  uint16_t i;
  
  if( kMatrixSerpentineLayout == false) {
    i = (y * MATRIX_WIDTH) + x;
  }

  if( kMatrixSerpentineLayout == true) {
    if( y & 0x01) {
      // Odd rows run backwards
      uint8_t reverseX = (MATRIX_WIDTH - 1) - x;
      i = (y * MATRIX_WIDTH) + reverseX;
    } else {
      // Even rows run forwards
      i = (y * MATRIX_WIDTH) + x;
    }
  }
  
  return i;
}

void loop() {
  //MultipleStream();
  //MultipleStream2();
  //MultipleStream3();
  //MultipleStream4();
  //MultipleStream5();
  //MultipleStream8();
  MultipleStream9();
  FastLED.show();
}
