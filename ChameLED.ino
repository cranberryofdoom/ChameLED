
#include "FastLED.h"
#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include "Adafruit_Pixie.h"
#include "SoftwareSerial.h"

#define SELECTOR_PIN 10
#define LED_PIN   6
#define PIXIE_PIN  9
#define NUM_PIXELS 3
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    60       // Change this to reflect the number of LEDs you have
#define BRIGHTNESS  255      // Set brightness here
#define UPDATES_PER_SECOND 100

SoftwareSerial pixieSerial(-1, PIXIE_PIN);
Adafruit_Pixie strip = Adafruit_Pixie(NUM_PIXELS, &pixieSerial);

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


// our RGB -> eye-recognized gamma color
byte gammatable[256];

void setup(void) {
  Serial.begin(9600);
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS)
    .setCorrection(TypicalLEDStrip) // cpt-city palettes have different color balance
    .setDither(BRIGHTNESS < 255);
  FastLED.setBrightness(BRIGHTNESS);
  
  pixieSerial.begin(115200);
  strip.setBrightness(BRIGHTNESS);  // Adjust as necessary to avoid blinding
  
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
}

uint16_t humanRed;
uint16_t humanGreen;
uint16_t humanBlue;
boolean isSensorLEDOff = true;

void loop(void) {
  uint16_t r, g, b, c, colorTemp, lux;
  
  tcs.setInterrupt(true);

  int thumbInput = digitalRead(SELECTOR_PIN);
  if (thumbInput == 0) {
    Serial.println("true");
    tcs.setInterrupt(false);
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
    humanRed = (uint16_t)gammatable[(int)red];
    humanGreen = (uint16_t)gammatable[(int)green];
    humanBlue = (uint16_t)gammatable[(int)blue];
  
    int pixelRed = (int)gammatable[(int)red];
    int pixelGreen = (int)gammatable[(int)green];
    int pixelBlue = (int)gammatable[(int)blue];
  
  //  Gammatable works better because it gives a higher
  //  variety of color
  //  humanRed = (int)red;
  //  humanGreen = (int)green;
  //  humanBlue = (int)blue;
  
  //  Serial.println(pixelRed);
  //  Serial.println(pixelGreen);
  //  Serial.println(pixelBlue);
  //
  //  strip.setPixelColor(0, 255, 255, 255);
  //  strip.show();
  //  delay(10);
  //  
    for(uint8_t i = 0 ; i < NUM_LEDS; i++) {
      leds[i] = CRGB(humanRed, humanGreen, humanBlue);
    }
    FastLED.show();
  }
}
