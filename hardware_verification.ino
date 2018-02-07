// Hardware Verification - This program tests whether a board connected to the kitting system
// is a Touchstone or a gateway. It also checks the status of all sensors and radio and
// returns the result over serial.

// 2/7/2018
// Developed by Akram Ali

// www.crtlabs.org

#include <RFM69.h>  //  https://github.com/LowPowerLab/RFM69
#include <SPI.h>
#include <Arduino.h>
#include <Wire.h> 
#include <CCS811.h> // https://github.com/AKstudios/CCS811-library
//#include <Adafruit_SHT31.h> //https://github.com/adafruit/Adafruit_SHT31
#include <Adafruit_Si7021.h> // https://github.com/adafruit/Adafruit_Si7021
#include <Adafruit_BMP280.h> // https://github.com/adafruit/Adafruit_BMP280_Library
#include <Adafruit_Sensor.h>  // https://github.com/adafruit/Adafruit_Sensor
#include <Adafruit_TSL2591.h> // https://github.com/adafruit/Adafruit_TSL2591_Library
#include <Adafruit_NeoPixel.h> // https://github.com/adafruit/Adafruit_NeoPixel

String FW_VERSION="0.5.7";

byte flag;
byte T, G, X;
byte r, t, g, l, v;

// define node parameters
#define NODEID        250  // test ID
#define GATEWAYID     1
#define NETWORKID     101
#define FREQUENCY     RF69_915MHZ //Match this with the version of your Moteino! (others: RF69_433MHZ, RF69_868MHZ)
#define ENCRYPTKEY    "Tt-Mh=SQ#dn#JY3_" //has to be same 16 characters/bytes on all nodes, not more not less!
#define IS_RFM69HW    //uncomment only for RFM69HW! Leave out if you have RFM69W!
#define PIN           6 // NeoPixel driver pin

// define objects
RFM69 radio;
Adafruit_BMP280 bme; // I2C
//Adafruit_SHT31 sht31 = Adafruit_SHT31();
Adafruit_Si7021 sensor = Adafruit_Si7021();
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591); // pass in a number for the sensor identifier (for your use later)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, PIN, NEO_GRB + NEO_KHZ800);  // number of pixels, digital pin, pixel flags


// define CCS811 global variables
#define ADDR      0x5B
#define WAKE_PIN  5
CCS811 ccs811_sensor;

char dataPacket[150];

void setup()
{
  pinMode(10, OUTPUT); // Radio SS pin set as output
  pinMode(9, OUTPUT);
  strip.begin(); // initialize neo pixels
  strip.show(); // Initialize all pixels to 'off'
  Serial.begin(115200);
  flag = 0;


  // check radio & sensors
  if(!radio.initialize(FREQUENCY,NODEID,NETWORKID)) // check radio
  {
    r = 0;
  }
  else
  {
    #ifdef IS_RFM69HW
      radio.setHighPower(); //uncomment only for RFM69HW!
    #endif
      radio.encrypt(ENCRYPTKEY);
    r = 1;
    flag++;
  }
  
  if(!ccs811_sensor.begin(uint8_t(ADDR), uint8_t(WAKE_PIN)))  // check CCS811 TVOC sensor
  {
    v = 0;
  }
  else
  {
    v = 1;
    flag++;
  }

  if(!sensor.begin())   // temperature and humidity sensor
  {
    t = 0;
  }
  else
  {
    t = 1;
    flag++;
  }

  if(!tsl.begin())  // Light sensor
  {
    l = 0;
  }
  else
  {
    l = 1;
    flag++;
  }

  if(!bme.begin())  // Pressure sensor
  {
    g = 0;
  }
  else
  {
    g = 1;
    flag++;
  }


  // check whether device is touchstone or gateway
  if(flag == 0) // Touchstone or gateway with nothing working
  {
    X = 0;
    colorWipe(strip.Color(255, 0, 0), 10); // Red
    strip.show();
  }
  else if(flag == 1)    // gateway detected OR touchstone detected with all sensors failed except radio
  {
    X = 1;
    digitalWrite(9, HIGH);
    colorWipe(strip.Color(255, 0, 0), 10); // Red
    strip.show();
  }
  else if(flag == 5)    // touchstone detected
  {
    X = 2;
    colorWipe(strip.Color(0, 255, 0), 10); // Green
    strip.show();
  }
  else if(flag > 1 && flag < 5)   // touchstone detected -- some sensors failed
  {
    X = 3;
    colorWipe(strip.Color(255, 70, 0), 10); // Yellowish-Orange
    strip.show();
  }


  // print output
  char output[30];
  output[0] = 0;  // first value of char array should be a 0
  snprintf(output, 30, "X:%d,t:%d,g:%d,l:%d,v:%d,r:%d\n", X,t,g,l,v,r);
  Serial.print(output);
  
}



void loop() 
{
  if(flag == 0)   // touchstone or gateway failed or unknown device
  {
    digitalWrite(9, HIGH);
    delay(700);
    digitalWrite(9, LOW);
    delay(700);
  }
}


// RGB LED
void colorWipe(uint32_t c, uint8_t wait)
{
  for(uint16_t i=0; i<strip.numPixels(); i++)
  {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}


// bruh
