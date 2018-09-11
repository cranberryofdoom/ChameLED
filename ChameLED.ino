#include <Adafruit_NeoPixel.h>
#include "FastLED.h"
#include <Wire.h>
#include "Adafruit_TCS34725.h"

#define SELECTOR_PIN 10
#define SKIRT_LED_PIN   6
#define TOP_LED_PIN 9
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_TOP_LEDS    8       // Change this to reflect the number of LEDs you have
#define NUM_SKIRT_LEDS    60       // Change this to reflect the number of LEDs you have
#define BRIGHTNESS  255      // Set brightness here


CRGB leds[NUM_SKIRT_LEDS];
   
/* Initialise with default values (int time = 2.4ms, gain = 1x) */
// Adafruit_TCS34725 tcs = Adafruit_TCS34725();

/* Initialise with specific int time and gain values */
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X);

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_TOP_LEDS, TOP_LED_PIN, NEO_GRB + NEO_KHZ800);


// our RGB -> eye-recognized gamma color
byte gammatable[256];
uint16_t prevMaxRGB[] = {255, 255, 255};
uint16_t maxRGB[] = {255, 255, 255};

void setup(void) {
  Serial.begin(9600);
  
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE, SKIRT_LED_PIN, COLOR_ORDER>(leds, NUM_SKIRT_LEDS)
    .setCorrection(TypicalLEDStrip) // cpt-city palettes have different color balance
    .setDither(BRIGHTNESS < 255);
  FastLED.setBrightness(BRIGHTNESS);

  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1); // halt!
  }
    
  pinMode(SELECTOR_PIN, INPUT_PULLUP);
  
  tcs.setInterrupt(true);

  // this helps convert RGB colors to what humans see
  for (int i=0; i<256; i++) {
    float x = i;
    x /= 255;
    x = pow(x, 2.5);
    x *= 255;
    gammatable[i] = x;      
  }
  pushColorToLEDsWithDelay(5);
}

void loop(void) {
  uint16_t r, g, b, c, colorTemp, lux;

//  printColors();
  
  tcs.setInterrupt(true);

  int thumbInput = digitalRead(SELECTOR_PIN);
  if (thumbInput == 1) {  
    breatheLEDs();
  }
  if (thumbInput == 0) {
    tcs.setInterrupt(false);
    tcs.getRawData(&r, &g, &b, &c);
    tcs.setInterrupt(true);
    colorTemp = tcs.calculateColorTemperature(r, g, b);
    lux = tcs.calculateLux(r, g, b);
    
    uint32_t sum = r; sum += g; sum += b;
  
    float red, green, blue;
  
    red = r; red /= sum;
    green = g; green /= sum;
    blue = b; blue /= sum;
    red *= 256; green *= 256; blue *= 256;

    uint16_t humanRed, humanGreen, humanBlue;
  
  // Apparently the gammatable version results in better color
    humanRed = (uint16_t)gammatable[(int)red];
    humanGreen = (uint16_t)gammatable[(int)green];
    humanBlue = (uint16_t)gammatable[(int)blue];

    float multiplier = 256/humanRed;

    if (humanGreen > humanRed) {
      multiplier = 256/humanGreen;
    } else if (humanBlue > humanGreen) {
      multiplier = 256/humanBlue;
    }
  
    float maxRed = humanRed * multiplier;
    float maxGreen = humanGreen * multiplier;
    float maxBlue = humanBlue * multiplier;

    if (maxRGB[0] != maxRed || maxRGB[1] != maxGreen || maxRGB[2] != maxBlue) {
      prevMaxRGB[0] = maxRGB[0];
      prevMaxRGB[1] = maxRGB[1];
      prevMaxRGB[2] = maxRGB[2];
      maxRGB[0] = maxRed;
      maxRGB[1] = maxGreen;
      maxRGB[2] = maxBlue;
      pushColorToLEDsWithDelay(5);
    } 
  }
}

void breatheLEDs() {
  for(uint8_t i = 0 ; i < NUM_SKIRT_LEDS; i++) {
    float breath = (exp(sin(millis()/2000.0*PI)) - 0.36787944)*108.0;
    FastLED.setBrightness(breath);
    FastLED.show();
  }
}

void pushColorToLEDsWithDelay(int delayMs) {
  for(uint8_t i = 0 ; i < NUM_TOP_LEDS; i++) {
    strip.setPixelColor(i, maxRGB[0], maxRGB[1], maxRGB[2]);
    strip.show();
  }
  FastLED.setBrightness(BRIGHTNESS);
  for(uint8_t i = 0 ; i < NUM_SKIRT_LEDS; i++) {
    leds[i] = CRGB(maxRGB[0], maxRGB[1], maxRGB[2]);
    FastLED.show();
    delay(delayMs);
  }
}

void printColors() {
  Serial.print(maxRGB[0]);
  Serial.print(" ");
  Serial.print(maxRGB[1]);
  Serial.print(" ");
  Serial.print(maxRGB[2]);
  Serial.println(" ");
}

