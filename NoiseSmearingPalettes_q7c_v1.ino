#include <FastLED.h>
#include <FastLEDMatrix.h>

//#define COLOR_DEPTH 24                  // known working: 24, 48 - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24
//const uint8_t MATRIX_WIDTH = 8;        // known working: 32, 64, 96, 128
//const uint8_t MATRIX_HEIGHT = 32;       // known working: 16, 32, 48, 64

#define LED_PIN        8
#define COLOR_ORDER    GRB
#define CHIPSET        WS2812B

#define MATRIX_WIDTH   8 // width of FastLED matrix
#define MATRIX_HEIGHT  32 // height of matrix
#define MATRIX_TYPE    (MTX_MATRIX_TOP + MTX_MATRIX_LEFT + MTX_MATRIX_COLUMNS + MTX_MATRIX_ZIGZAG) // matrix layout flags, add together as needed

cFastLEDSingleMatrix<MATRIX_WIDTH, MATRIX_HEIGHT, MATRIX_TYPE> matrix;

// NoiseSmearing by Stefan Petrick: https://gist.github.com/StefanPetrick/9ee2f677dbff64e3ba7a

// Timed playlist code is from Mark Kriegsman's TimedPlaylist demo here: https://gist.github.com/kriegsman/841c8cd66ed40c6ecaae

// Palette cross-fading is from Mark Kriegsman's demo here: https://gist.github.com/kriegsman/1f7ccbbfa492a73c015e

byte CentreX =  (MATRIX_WIDTH / 2) - 1;
byte CentreY = (MATRIX_HEIGHT / 2) - 1;

#define NUM_LEDS (MATRIX_WIDTH * MATRIX_HEIGHT)
CRGB *leds;
CRGB leds2[NUM_LEDS];

// The coordinates for 3 16-bit noise spaces.
#define NUM_LAYERS 1

uint8_t x[NUM_LAYERS];
uint8_t y[NUM_LAYERS];
uint8_t z[NUM_LAYERS];
uint8_t scale_x[NUM_LAYERS];
uint8_t scale_y[NUM_LAYERS];

uint8_t noise[NUM_LAYERS][MATRIX_WIDTH][MATRIX_HEIGHT];

uint8_t noisesmoothing;

// List of patterns to cycle through.  Each is defined as a separate function below.

typedef void(*SimplePattern)();
typedef SimplePattern SimplePatternList [];
typedef struct {
  SimplePattern mPattern;
  uint8_t mTime;
} PatternAndTime;
typedef PatternAndTime PatternAndTimeList [];

// These times are in seconds, but could be changed to milliseconds if desired;
// there's some discussion further below.

// forward declaration of functions, to fix Arduino build errors
void MultipleStream();
void MultipleStream2();
void MultipleStream3();
void MultipleStream4();
void MultipleStream5();
void MultipleStream8();
void BasicVariablesSetup();
void RestartPlaylist();
void nextPalette();
void PaletteSmear();
void nextPattern();

const PatternAndTimeList gPlaylist = {
  { MultipleStream, 5 },
  { MultipleStream2, 10 }, // nice and slow, three dots
  { MultipleStream3, 10 }, // fire across the midddle: 8 dots in a line across the middle, slow noise smear
  { MultipleStream4, 5 }, // a single dot in the middle that rapidly changes color, with very fast noise smearing
  { MultipleStream5, 5 }, // fire across the bottom: 8 dots in a line across the bottom, slow noise smear
  { MultipleStream8, 10 },
};

// If you want the playlist to loop forever, set this to true.
// If you want the playlist to play once, and then stay on the final pattern
// until the playlist is reset, set this to false.
bool gLoopPlaylist = true;

CRGBPalette16 currentPalette( CRGB::Black);

CRGBPalette16 targetPalette( RainbowColors_p );

void setup() {
  //Serial.begin(115200);

  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(matrix[0], matrix.Size());
  FastLED.setBrightness(127);
  FastLED.clear(true);


  
  BasicVariablesSetup();

  RestartPlaylist();
}

uint8_t gCurrentTrackNumber = 0; // Index number of which pattern is current

bool gRestartPlaylistFlag = false;

void loop() {
//  leds = (CRGB*) backgroundLayer.backBuffer();
    
  EVERY_N_SECONDS(5) {
    nextPalette();
  }

  // Crossfade current palette slowly toward the target palette
  //
  // Each time that nblendPaletteTowardPalette is called, small changes
  // are made to currentPalette to bring it closer to matching targetPalette.
  // You can control how many changes are made in each call:
  //   - the default of 24 is a good balance
  //   - meaningful values are 1-48.  1=veeeeeeeery slow, 48=quickest
  //   - "0" means do not change the currentPalette at all; freeze

  uint8_t maxChanges = 12;
  nblendPaletteTowardPalette( currentPalette, targetPalette, maxChanges);

  // Call the current pattern function once, updating the 'leds' array
  //gPlaylist[gCurrentTrackNumber].mPattern();

  PaletteSmear();
  //MultipleStream();
  //MultipleStream2();
  //MultipleStream3();
  //MultipleStream4();
  //MultipleStream5();
  //MultipleStream8();

//  backgroundLayer.swapBuffers();
    
  // Here's where we do two things: switch patterns, and also set the
  // 'virtual timer' for how long until the NEXT pattern switch.
  //
  // Instead of EVERY_N_SECONDS(10) { nextPattern(); }, we use a special
  // variation that allows us to get at the pattern timer object itself,
  // and change the timer period every time we change the pattern.
  //
  // You could also do this with EVERY_N_MILLISECONDS_I and have the
  // times be expressed in milliseconds instead of seconds.
  {
    EVERY_N_SECONDS_I(patternTimer, gPlaylist[gCurrentTrackNumber].mTime) {
      nextPattern();
      patternTimer.setPeriod(gPlaylist[gCurrentTrackNumber].mTime);
    }

    // Here's where we handle restarting the playlist if the 'reset' flag
    // has been set. There are a few steps:
    if (gRestartPlaylistFlag) {

      // Set the 'current pattern number' back to zero
      gCurrentTrackNumber = 0;

      // Set the playback duration for this patter to it's correct time
      patternTimer.setPeriod(gPlaylist[gCurrentTrackNumber].mTime);
      // Reset the pattern timer so that we start marking time from right now
      patternTimer.reset();

      // Finally, clear the gRestartPlaylistFlag flag
      gRestartPlaylistFlag = false;
    }
  }
}
/*
void setupGrayscalePalette() {
  targetPalette = CRGBPalette16(CRGB::Black, CRGB::White);
}

void setupIcePalette() {
  targetPalette = CRGBPalette16(CRGB::Black, CRGB::Blue, CRGB::Aqua, CRGB::White);
}

void setupRandomPalette() {
  targetPalette = CRGBPalette16(
                    CHSV(random8(), 255, 0),
                    CHSV(random8(), 255, 0),
                    CHSV(random8(), 255, 255),
                    CHSV(random8(), 255, 255),

                    CHSV(random8(), 255, 0),
                    CHSV(random8(), 255, 255),
                    CHSV(random8(), 255, 255),
                    CHSV(random8(), 255, 0),

                    CHSV(random8(), 255, 0),
                    CHSV(random8(), 255, 255),
                    CHSV(random8(), 255, 255),
                    CHSV(random8(), 255, 0),

                    CHSV(random8(), 255, 255),
                    CHSV(random8(), 255, 255),
                    CHSV(random8(), 255, 0),
                    CHSV(random8(), 255, 0));
}
*/
void nextPalette()
{
  static uint8_t paletteIndex = 0;
  paletteIndex++;

  switch (paletteIndex) {
    case  1: targetPalette = RainbowStripeColors_p; break;
    case  2: targetPalette = OceanColors_p; break;
    case  3: targetPalette = CloudColors_p; break;
    case  4: targetPalette = ForestColors_p; break;
    case  5: targetPalette = HeatColors_p; break;
    case  6: targetPalette = PartyColors_p; break;
    case  7: targetPalette = LavaColors_p; break;
    //case  8: setupIcePalette(); break;
    //case  9: setupRandomPalette(); break;
    //case 10: setupGrayscalePalette(); break;
    case  0:
    default: paletteIndex = 0; targetPalette = RainbowColors_p; break;
  }
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number
  gCurrentTrackNumber = gCurrentTrackNumber + 1;

  // If we've come to the end of the playlist, we can either
  // automatically restart it at the beginning, or just stay at the end.
  if (gCurrentTrackNumber == ARRAY_SIZE(gPlaylist)) {
    if (gLoopPlaylist == true) {
      // restart at beginning
      gCurrentTrackNumber = 0;
    }
    else {
      // stay on the last track
      gCurrentTrackNumber--;
    }
  }
}

void RestartPlaylist()
{
  gRestartPlaylistFlag = true;
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

uint16_t XY( uint8_t x, uint8_t y) {
  uint16_t i;
  i = (y * MATRIX_WIDTH) + x;
  return i;
}

void DimAll(byte value)
{
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].nscale8(value);
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

void MoveX(byte delta) {

  //CLS2();

  for (int y = 0; y < MATRIX_HEIGHT; y++) {

    for (int x = 0; x < MATRIX_WIDTH - delta; x++) {
      leds2[XY(x, y)] = leds[XY(x + delta, y)];
    }
    for (int x = MATRIX_WIDTH - delta; x < MATRIX_WIDTH; x++) {
      leds2[XY(x, y)] = leds[XY(x + delta - MATRIX_WIDTH, y)];
    }
  }


  //CLS();

  // write back to leds
  for (uint8_t y = 0; y < MATRIX_HEIGHT; y++) {
    for (uint8_t x = 0; x < MATRIX_WIDTH; x++) {
      leds[XY(x, y)] = leds2[XY(x, y)];
    }
  }
}


void MoveY(byte delta) {

  //CLS2();

  for (int x = 0; x < MATRIX_WIDTH; x++) {

    for (int y = 0; y < MATRIX_HEIGHT - delta; y++) {
      leds2[XY(x, y)] = leds[XY(x, y + delta)];
    }
    for (int y = MATRIX_HEIGHT - delta; y < MATRIX_HEIGHT; y++) {
      leds2[XY(x, y)] = leds[XY(x, y + delta - MATRIX_HEIGHT)];
    }
  }


  //CLS();

  // write back to leds
  for (uint8_t y = 0; y < MATRIX_HEIGHT; y++) {
    for (uint8_t x = 0; x < MATRIX_WIDTH; x++) {
      leds[XY(x, y)] = leds2[XY(x, y)];
    }
  }
}

void MoveFractionalNoiseX(byte amt = 16) {

  // move delta pixelwise
  for (int y = 0; y < MATRIX_HEIGHT; y++) {

    uint16_t amount = noise[0][0][y] * amt;
    byte delta = 31 - (amount / 256);
    byte fractions = amount - (delta * 256);

    for (int x = 0; x < MATRIX_WIDTH - delta; x++) {
      leds2[XY(x, y)] = leds[XY(x + delta, y)];
    }
    for (int x = MATRIX_WIDTH - delta; x < MATRIX_WIDTH; x++) {
      leds2[XY(x, y)] = leds[XY(x + delta - MATRIX_WIDTH, y)];
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
      PixelA = leds2[XY(x, y)];
      PixelB = leds2[XY(x - 1, y)];

      PixelA %= 255 - fractions;
      PixelB %= fractions;


      leds[XY(x, y)] = PixelA + PixelB;
      //leds2[XY(x, y)] = 300;
    }

    PixelA = leds2[XY(0, y)];
    PixelB = leds2[XY(MATRIX_WIDTH - 1, y)];

    PixelA %= 255 - fractions;
    PixelB %= fractions;
    /*
      PixelB.r = dim8_raw(PixelB.r);
      PixelB.g = dim8_raw(PixelB.g);
      PixelB.b = dim8_raw(PixelB.b);
    */


    leds[XY(0, y)] = PixelA + PixelB;

  }

}


void MoveFractionalNoiseY(byte amt = 16) {

  // move delta pixelwise
  for (int x = 0; x < MATRIX_WIDTH; x++) {

    uint16_t amount = noise[0][x][0] * amt;
    byte delta = 31 - (amount / 256);
    byte fractions = amount - (delta * 256);

    for (int y = 0; y < MATRIX_WIDTH - delta; y++) {
      leds2[XY(x, y)] = leds[XY(x, y + delta)];
    }
    for (int y = MATRIX_WIDTH - delta; y < MATRIX_WIDTH; y++) {
      leds2[XY(x, y)] = leds[XY(x, y + delta - MATRIX_WIDTH)];
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
      PixelA = leds2[XY(x, y)];
      PixelB = leds2[XY(x, y - 1)];

      PixelA %= 255 - fractions;
      PixelB %= fractions;


      leds[XY(x, y)] = PixelA + PixelB;
      //leds2[XY(x, y)] = 300;
    }

    PixelA = leds2[XY(x, 0)];
    PixelB = leds2[XY(x, MATRIX_WIDTH - 1)];

    PixelA %= 255 - fractions;
    PixelB %= fractions;
    /*
      PixelB.r = dim8_raw(PixelB.r);
      PixelB.g = dim8_raw(PixelB.g);
      PixelB.b = dim8_raw(PixelB.b);
    */


    leds[XY(x, 0)] = PixelA + PixelB;

  }

}



void CLS()
{
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = 0;
  }
}

void CLS2() {

  for (int y = 0; y < NUM_LEDS; y++) {
    leds2[y] = 0;
  }
}

void MultipleStream() {

  //CLS();
  DimAll(249);

  // yellow circle
  byte xx = 4 + sin8( millis() / 10) / 10;
  byte yy = 4 + cos8( millis() / 10) / 10;
  leds[XY( xx, yy)] = 0xFFFF00;

  // red in a figure eight
  xx = 8 + sin8( millis() / 46) / 16;
  yy = 8 + cos8( millis() / 15) / 16;
  leds[XY( xx, yy)] = 0xFF0000;

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
  leds[XY( xx, yy)] += 0x0000FF;

  xx = 8 + sin8( millis() / 10) / 16;
  yy = 8 + cos8( millis() / 7) / 16;
  leds[XY( xx, yy)] += 0xFF0000;

  leds[XY( 15, 15)] += 0xFFFF00;

  x[0] += 1000;
  y[0] += 1000;
  z[0] += 1000;
  scale_x[0] = 4000;
  scale_y[0] = 4000;
  FillNoise(0);

  MoveX(3);
  MoveFractionalNoiseY(4);


  MoveY(3);
  MoveFractionalNoiseX(4);
}


void MultipleStream3() {

  //CLS();
  DimAll(235);


  for (uint8_t i = 3; i < 32; i = i + 4) {
    leds[XY( i, 15)] += CHSV(i * 2, 255, 255);
  }

  // Noise
  x[0] += 1000;
  y[0] += 1000;
  z[0] += 1000;
  scale_x[0] = 4000;
  scale_y[0] = 4000;
  FillNoise(0);

  MoveX(3);
  MoveFractionalNoiseY(4);


  MoveY(3);
  MoveFractionalNoiseX(4);
}

void MultipleStream4() {

  //CLS();
  DimAll(235);

  leds[XY( 15, 15)] += CHSV(millis(), 255, 255);


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


void MultipleStream5() {

  //CLS();
  DimAll(235);


  for (uint8_t i = 3; i < 32; i = i + 4) {
    leds[XY( i, 31)] += CHSV(i * 2, 255, 255);
  }

  // Noise
  x[0] += 1000;
  y[0] += 1000;
  z[0] += 1000;
  scale_x[0] = 4000;
  scale_y[0] = 4000;
  FillNoise(0);

  MoveX(3);
  MoveFractionalNoiseY(4);


  MoveY(4);
  MoveFractionalNoiseX(4);
}

void MultipleStream8() {

  //CLS();
  DimAll(230);

  for (uint8_t y = 1; y < 32; y = y + 6) {
    for (uint8_t x = 1; x < 32; x = x + 6) {

      leds[XY( x, y)] += CHSV((x * y) / 4, 255, 255);
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
  MoveFractionalNoiseX(4);

  MoveY(3);
  MoveFractionalNoiseY(4);

}

void PaletteSmear() {
  DimAll(170);

  // draw a rainbow color palette
  for (uint8_t y = 0; y < MATRIX_HEIGHT; y++) {
    for (uint8_t x = 0; x < MATRIX_WIDTH; x++) {
      leds[XY(x, y)] += ColorFromPalette(currentPalette, x * 8, y * 8 + 7, LINEARBLEND);
    }
  }

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
