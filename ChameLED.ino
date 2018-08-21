
#include "FastLED.h"
#include <Wire.h>
#include "Adafruit_TCS34725.h"


#define DATA_PIN   6
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    60       // Change this to reflect the number of LEDs you have
#define BRIGHTNESS  255      // Set brightness here

CRGB leds[NUM_LEDS];

/* Example code for the Adafruit TCS34725 breakout library */

/* Connect SCL    to analog 5
   Connect SDA    to analog 4
   Connect VDD    to 3.3V DC
   Connect GROUND to common ground */
   
/* Initialise with default values (int time = 2.4ms, gain = 1x) */
// Adafruit_TCS34725 tcs = Adafruit_TCS34725();

/* Initialise with specific int time and gain values */
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X);

int greenPin = 6;
int bluePin = 9;
int redPin = 10;
int thumbButton = 12;

// our RGB -> eye-recognized gamma color
byte gammatable[256];


void setup(void) {
  Serial.begin(9600);
  delay(3000);

  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS)
    .setCorrection(TypicalLEDStrip) // cpt-city palettes have different color balance
    .setDither(BRIGHTNESS < 255);

  FastLED.setBrightness(BRIGHTNESS);

  // this helps convert RGB colors to what humans see
  for (int i=0; i<256; i++) {
    float x = i;
    x /= 255;
    x = pow(x, 2.5);
    x *= 255;
    gammatable[i] = x;      
  }
}

void loop(void) {
  uint16_t r, g, b, c, colorTemp, lux;

  tcs.getRawData(&r, &g, &b, &c);
  colorTemp = tcs.calculateColorTemperature(r, g, b);
  lux = tcs.calculateLux(r, g, b);
  
  uint32_t sum = r;
  sum += g;
  sum += b;

  float red, green, blue;

  red = r; red /= sum;
  green = g; green /= sum;
  blue = b; blue /= sum;
  red *= 256; green *= 256; blue *= 256;

  // Apparently the gammatable version results in better color
  int humanRed = (int)gammatable[(int)red];
  int humanGreen = (int)gammatable[(int)green];
  int humanBlue = (int)gammatable[(int)blue];
    
  Serial.print((int)red, HEX);
  Serial.print((int)green, HEX);
  Serial.print((int)blue, HEX);
  
  Serial.println(" ");
  
  Serial.print(humanRed);
  Serial.print(" ");
  Serial.print(humanGreen);
  Serial.print(" ");
  Serial.print(humanBlue);
  
  Serial.println(" ");
  
  Serial.print(red);
  Serial.print(" ");
  Serial.print(green);
  Serial.print(" ");
  Serial.print(blue);
  
  Serial.println(" ");

  for(int i = 0 ; i < NUM_LEDS; i++) {
    leds[i] = CRGB(humanRed, humanGreen, humanBlue);
  }
  FastLED.show();
}
