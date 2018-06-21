#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#include <EEPROM.h>

#define PIN_RING 2
#define PIN_LEFT 3
#define PIN_RIGHT 9
#define NUM_LEDS 19

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN_RING, NEO_GRBW + NEO_KHZ800);

Adafruit_NeoPixel stripLeft = Adafruit_NeoPixel(NUM_LEDS, PIN_LEFT, NEO_GRBW + NEO_KHZ800);
Adafruit_NeoPixel stripRight = Adafruit_NeoPixel(NUM_LEDS, PIN_RIGHT, NEO_GRBW + NEO_KHZ800);
  
// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

uint8_t prog = 0;
int nb_prog = 10;

uint32_t lastSideColor = 0;

void setup() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code

  #define max_led 100

  strip.begin();
  stripLeft.begin();
  stripRight.begin();
  
  strip.show(); // Initialize all pixels to 'off'
  stripLeft.show();
  stripRight.show();

  setSideColor(strip.Color(100, 0, 200, 0));

  prog = EEPROM.read(0);
  if (prog >= nb_prog)
    prog = 0;
  else
    prog++;
  EEPROM.write(0,prog);
  //prog = 8;
  Serial.begin(115200);
  Serial.println(prog);
}


void loop() {
  
  switch (prog) {
    case 0:
      setAllC(strip.Color(0, 0, 255, 0));
      break;
    case 1:
      // TODO changer couleurs
       colorWipe(strip.Color(0, 0, 255), 50); // Red
       colorWipe(strip.Color(255, 255, 255), 50); // Green
       colorWipe(strip.Color(255, 0, 0), 50); // Blue
       break;
     case 2:
      rainbow(20);
      break;
     case 3:
      rainbow(5);
      break;
     case 4:
      rainbowCycle(20);
      break;
     case 5:
      rainbowCycle(5);
      break;
     case 6:
      theaterChaseRainbow(50);
      break;
     case 7:
      // 380 + changement couleur
      CylonBounce(255, 0, 0, 4, 30, 50);
      break;
     case 8:
      // changer la couleur 
      RunningLights(0,0,255, 50);
      break;
     case 9:
      Fire(55,120,15);
      break;
     case 10:
        for (int color=0; color<255; color++) {
            for (int i=0; i<strip.numPixels(); i++) {
              strip.setPixelColor(i, Wheel(color));
             }
          strip.show();
          delay(10);
        }
      break;
     case 11:
      colorWave(30);
      colorWipe(strip.Color(0,0,0), 100); // Black
      rainbow(15);
      colorWipe(strip.Color(0,0,0), 100); // Black
      rainbowCycle(15);
      colorWipe(strip.Color(0,0,0), 100); // Black
      colorWave(30);
      break;
  }
  strip.show();
}

void setSideColor(uint32_t c) {

  if(c != lastSideColor) {
    lastSideColor = c;
    
    setAll(stripLeft, c);
    setAll(stripRight, c);
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//https://gist.github.com/dougalcampbell/7243998
/**
 *      ^   ^   ^  
 * ~~~~~ ColorWave ~~~~~
 *        V   V   V   
 */
void colorWave(uint8_t wait) {
  int i, j, stripsize, cycle;
  float ang, rsin, gsin, bsin, offset;

  static int tick = 0;
  
  stripsize = strip.numPixels();
  cycle = stripsize * 25; // times around the circle...

  while (++tick % cycle) {
    offset = map2PI(tick);

    for (i = 0; i < stripsize; i++) {
      ang = map2PI(i) - offset;
      rsin = sin(ang);
      gsin = sin(2.0 * ang / 3.0 + map2PI(int(stripsize/6)));
      bsin = sin(4.0 * ang / 5.0 + map2PI(int(stripsize/3)));
      strip.setPixelColor(i, strip.Color(trigScale(rsin), trigScale(gsin), trigScale(bsin)));
    }

    strip.show();
    delay(wait);
  }

}

/**
 * Scale a value returned from a trig function to a byte value.
 * [-1, +1] -> [0, 254] 
 * Note that we ignore the possible value of 255, for efficiency,
 * and because nobody will be able to differentiate between the
 * brightness levels of 254 and 255.
 */
byte trigScale(float val) {
  val += 1.0; // move range to [0.0, 2.0]
  val *= 127.0; // move range to [0.0, 254.0]

  return int(val) & 255;
}

/**
 * Map an integer so that [0, striplength] -> [0, 2PI]
 */
float map2PI(int i) {
  return PI*2.0*float(i) / float(strip.numPixels());
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void setAll(Adafruit_NeoPixel s, uint32_t c) {
  for(int i = 0; i < NUM_LEDS; i++ ) {
    s.setPixelColor(i, c);
  }
  s.show();
}

void setAllC(uint32_t c) {
  for(int i = 0; i < NUM_LEDS; i++ ) {
    strip.setPixelColor(i, c);
  }
  strip.show();
}

void CylonBounce(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay){

  for(int i = 0; i < NUM_LEDS-EyeSize-2; i++) {
    setAllC(strip.Color(0, 0, 0, 0));
    strip.setPixelColor(i, red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) {
      strip.setPixelColor(i+j, red, green, blue); 
    }
    strip.setPixelColor(i+EyeSize+1, red/10, green/10, blue/10);
    strip.show();
    delay(SpeedDelay);
  }

  delay(ReturnDelay);

  for(int i = NUM_LEDS-EyeSize-2; i > 0; i--) {
    setAllC(strip.Color(0, 0, 0, 0));
    strip.setPixelColor(i, red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) {
      strip.setPixelColor(i+j, red, green, blue); 
    }
    strip.setPixelColor(i+EyeSize+1, red/10, green/10, blue/10);
    strip.show();
    delay(SpeedDelay);
  }
  
  delay(ReturnDelay);
}

void RunningLights(byte red, byte green, byte blue, int WaveDelay) {
  int Position=0;
  
  for(int i=0; i<NUM_LEDS*2; i++)
  {
      Position++; // = 0; //Position + Rate;
      for(int i=0; i<NUM_LEDS; i++) {
        // sine wave, 3 offset waves make a rainbow!
        //float level = sin(i+Position) * 127 + 128;
        //setPixel(i,level,0,0);
        //float level = sin(i+Position) * 127 + 128;
        strip.setPixelColor(i,((sin(i+Position) * 127 + 128)/255)*red,
                   ((sin(i+Position) * 127 + 128)/255)*green,
                   ((sin(i+Position) * 127 + 128)/255)*blue);
      }
      
      strip.show();
      delay(WaveDelay);
  }
}

void Fire(int Cooling, int Sparking, int SpeedDelay) {
  static byte heat[NUM_LEDS];
  int cooldown;
  
  // Step 1.  Cool down every cell a little
  for( int i = 0; i < NUM_LEDS; i++) {
    cooldown = random(0, ((Cooling * 10) / NUM_LEDS) + 2);
    
    if(cooldown>heat[i]) {
      heat[i]=0;
    } else {
      heat[i]=heat[i]-cooldown;
    }
  }
  
  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for( int k= NUM_LEDS - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }
    
  // Step 3.  Randomly ignite new 'sparks' near the bottom
  if( random(255) < Sparking ) {
    int y = random(7);
    heat[y] = heat[y] + random(160,255);
    //heat[y] = random(160,255);
  }

  // Step 4.  Convert heat to LED colors
  for( int j = 0; j < NUM_LEDS; j++) {
    setPixelHeatColor(j, heat[j] );
  }

  strip.show();
  delay(SpeedDelay);
}

void setPixelHeatColor (int Pixel, byte temperature) {
  // Scale 'heat' down from 0-255 to 0-191
  byte t192 = round((temperature/255.0)*191);
 
  // calculate ramp up from
  byte heatramp = t192 & 0x3F; // 0..63
  heatramp <<= 2; // scale up to 0..252
 
  // figure out which third of the spectrum we're in:
  if( t192 > 0x80) {                     // hottest
    strip.setPixelColor(Pixel, 255, 255, heatramp);
  } else if( t192 > 0x40 ) {             // middle
    strip.setPixelColor(Pixel, 255, heatramp, 0);
  } else {                               // coolest
    strip.setPixelColor(Pixel, heatramp, 0, 0);
  }
}
