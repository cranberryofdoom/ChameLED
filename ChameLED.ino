#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include "Adafruit_TCS34725.h"
#define PIN 8

Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, PIN, NEO_GRB + NEO_KHZ800);


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
int led = 7;
int thumbButton = 12;

// our RGB -> eye-recognized gamma color
byte gammatable[256];


void setup(void) {
  Serial.begin(9600);
  
  pinMode(greenPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(led, OUTPUT);
  pinMode(thumbButton, INPUT_PULLUP);

  // this helps convert RGB colors to what humans see
  for (int i=0; i<256; i++) {
    float x = i;
    x /= 255;
    x = pow(x, 2.5);
    x *= 255;
    gammatable[i] = x;      
  }

  strip.begin();
  strip.setBrightness(255);
  strip.show();
}

boolean isSensorLEDOff = true;

void loop(void) {
  uint16_t r, g, b, c, colorTemp, lux;
  
  tcs.setInterrupt(true);
  int thumbInput = digitalRead(thumbButton);

  Serial.println(thumbInput);

  if (thumbInput == 0) {
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
    float humanRed = gammatable[(int)red];
    float humanGreen = gammatable[(int)green];
    float humanBlue = gammatable[(int)blue];
  
    
    // For anode LEDs 255 is no brightness and 0 is max brightness
    float anodeLEDRed = 255 - red;
    float anodeLEDGreen = 255 - green;
    float anodeLEDBlue = 255 - blue;
    
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
    
    Serial.print("Color Temp: "); Serial.print(colorTemp, DEC); Serial.print(" K - ");
    Serial.print("Lux: "); Serial.print(lux, DEC); Serial.print(" - ");
    Serial.print("R: "); Serial.print(r, DEC); Serial.print(" ");
    Serial.print("G: "); Serial.print(g, DEC); Serial.print(" ");
    Serial.print("B: "); Serial.print(b, DEC); Serial.print(" ");
    Serial.print("C: "); Serial.print(c, DEC); Serial.print(" ");
    Serial.println(" ");
    Serial.println(" ");
  
    analogWrite(greenPin, anodeLEDGreen);
    analogWrite(redPin, anodeLEDRed);
    analogWrite(bluePin, anodeLEDBlue);
  
    strip.setPixelColor (0, strip.Color(humanRed, humanGreen, humanBlue));
    strip.show();
  }
}
