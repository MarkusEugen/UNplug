/*
 *  solarUSBControllerLib v1
 *
 *  --- To compile set the board type to Arduino Nano or Spark Core ---
 *
 *  Utilities for solarUSBController board - 
 *  Original code by Markus E. Loeffler June 2014
 *
 *  This is free software. You can redistribute it and/or modify it under
 *  the terms of Creative Commons Attribution 3.0 United States License. 
 *  To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/us/ 
 *  or send a letter to Creative Commons, 171 Second Street, Suite 300, San Francisco, California, 94105, USA.
 *
 */

#ifdef ARDUINO
#include <OneWire.h>
#include <DallasTemperature.h>
#else
#include "OneWire/OneWire.h"
#include "spark-dallas-temperature/spark-dallas-temperature.h"
#endif

#ifdef ARDUINO
#define mainPin 8
#define buzzerPin 4
#define DS18S20_Pin 9
#define ChargerPin 11
#define inverterPin 10
#define cityACPin 3  //int.5
#define inverterACPin 2 //int.4
#define solarVPin A4
#define solarAPin A5
#define battVPin A6
#define battAPin A7
#else
#define mainPin D2
#define buzzerPin D3
#define DS18S20_Pin D4
#define ChargerPin A7
#define inverterPin A6
#define cityACPin A5  //int.5
#define inverterACPin A4 //int.4
#define solarVPin A3
#define solarAPin A2
#define battVPin A1
#define battAPin A0
#endif

#define V_SPARK       3.3

#ifdef ARDUINO
const float zeroAbOffset = 512.;
const float zeroAsOffset = 512.;
#else
const float zeroAbOffset = 2048.;
const float zeroAsOffset = 2048.;
unsigned char wifiMode = 1;
#endif


OneWire ds(DS18S20_Pin);
DallasTemperature sensors(&ds);
DeviceAddress thermometer;

void setBuzzer(uint8_t value)
{
  digitalWrite(buzzerPin, value);
}

void setCharger(uint8_t value)
{
  analogWrite(ChargerPin, value);
}

void setDimmer(uint8_t value)
{
  analogWrite(inverterPin, value);
}

void setRelay(uint8_t state)
{
  digitalWrite(mainPin, state);
}

float getVs()
{
#ifdef ARDUINO
  return V_SPARK*(analogRead(solarVPin))/1023.*(28800./1800.);// best resolution is +/-0.25 V
#else
  return V_SPARK*(analogRead(solarVPin))/4095.*(28800./1800.);// best resolution is +/-0.064 V
#endif
}

float	getVb()
{
#ifdef ARDUINO
  return V_SPARK*(analogRead(battVPin))/1023.*(12300./1300.); // best resolution is +/-0.03 V
#else
  return V_SPARK*(analogRead(battVPin))/4095.*(12300./1300.); // best resolution is +/-0.0076 V
#endif
}

float	getAs()
{
#ifdef ARDUINO
  return ((V_SPARK*(analogRead(solarAPin) -zeroAsOffset)/1023.)/.03);
#else
  return ((V_SPARK*(analogRead(solarAPin) -zeroAsOffset)/4095.)/.03);
#endif
}

float	getAb()
{
#ifdef ARDUINO
  return ((V_SPARK*(analogRead(battAPin) -zeroAbOffset)/1023.)/ -.03);  // should be 0.03 for 50A best resolution is +/- 0.1 A
#else
  return ((V_SPARK*(analogRead(battAPin) -zeroAbOffset)/4095.)/ -.03);  // should be 0.03 for 50A best resolution is +/- 0.027 A
#endif
}

void cityZeroCross()
{
  // interrupt code goes here;
}

void inverterZeroCross()
{
  // interrupt code goes here;
}

void firstOneWireDevices()
{
  byte i;
  byte present = 0;
  byte data[12];

  Serial.print("Temp sensor?\n");
  while(ds.search(thermometer)) {
    Serial.print("\n\rFound Temp sensor address:\n");
    for( i = 0; i < 8; i++) {
      Serial.print("0x");
      if (thermometer[i] < 16) {
        Serial.print('0');
      }
    }
    if ( OneWire::crc8( thermometer, 7) != thermometer[7]) {
      Serial.print("CRC not valid!\n");
      return;
    }
  }
  ds.reset_search();
  return;
}

float getTemp()
{
  sensors.requestTemperatures();
  float tempC = sensors.getTempC(thermometer);
#ifdef ARDUINO
  if(DEVICE_DISCONNECTED != tempC)
#else
    if(DEVICE_DISCONNECTED_C != tempC)
#endif
      Serial.println("readTemp failed");
  return tempC;
}


void setup()
{
  Serial.begin(115200);
  firstOneWireDevices();
  sensors.begin();
  sensors.setResolution(thermometer, 10);

  pinMode(inverterPin, OUTPUT);
  pinMode(mainPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
#ifdef ARDUINO
  analogReference(EXTERNAL);
  pinMode(cityACPin, INPUT_PULLUP);
  pinMode(inverterACPin, INPUT_PULLUP);
#else
  pinMode(cityACPin, INPUT_PULLUP);
  pinMode(inverterACPin, INPUT_PULLUP);
#endif
  pinMode(ChargerPin, OUTPUT);
  digitalWrite(buzzerPin,HIGH);  
#ifdef ARDUINO
  attachInterrupt(1, cityZeroCross, FALLING);
  attachInterrupt(0, inverterZeroCross, FALLING);
#else
  attachInterrupt(cityACPin, cityZeroCross, FALLING);
  attachInterrupt(inverterACPin, inverterZeroCross, FALLING);
#endif

  setDimmer(0);
  setBuzzer(0);
  setCharger(0);
  setRelay(0);
}

void loop()
{
    Serial.print("Temp (C): "); Serial.print(getTemp());
    Serial.print(" As: "); Serial.print(getAs());
    Serial.print(" Vs: "); Serial.print(getVs());
    Serial.print(" Ab: "); Serial.print(getAb());
    Serial.print(" Vb: "); Serial.println(getVb());
    delay(1000);
}


