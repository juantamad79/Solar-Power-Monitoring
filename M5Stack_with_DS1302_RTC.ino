// Example sketch for interfacing with the DS1302 timekeeping chip.
//
// Copyright (c) 2009, Matt Sparks
// All rights reserved.
//
// http://quadpoint.org/projects/arduino-ds1302
#include <stdio.h>
#include <DS1302.h>
#include <TimeLib.h>
#include <M5Stack.h>
#include "utility/MPU9250.h"


namespace {

// Set the appropriate digital I/O pin connections. These are the pin
// assignments for the Arduino as well for as the DS1302 chip. See the DS1302
// datasheet:
//
//   http://datasheets.maximintegrated.com/en/ds/DS1302.pdf
const int kCePin   = 18;                    // Chip Enable
const int kIoPin   = 1;                    // Input/Output
const int kSclkPin = 2;                    // Serial Clock
//#define DS1302_GND_PIN 18                   // Gnd Pin for RTC
//#define DS1302_VCC_PIN 23                   // VCC Pin for RTC

const int enable_rtc_time_set = 0;          // enable this if you want to set new time/date in RTC
int sensorPin = A0;                         // select the input pin for the voltage measurement signal
int sensorValue = 0;                        // variable to store the value coming from the sensor
float voltageValue = 0;                     // Initial value for voltage measurement
#define V_REF 5090                          // Reference value 
#define precision_val 16

DS1302 rtc(kCePin, kIoPin, kSclkPin);       // Create a DS1302 object.

void printTime() {                          // Function to display current time/date in serial monitor
  // Get the current time and date from the chip.
  time_t timeNow = now();
  // Name the day of the week.
  //const String day = dayAsString(t.day);

  // Format the time and date and insert into the temporary buffer.
  char buf[50];
  snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
           year(timeNow), month(timeNow), day(timeNow),
           hour(timeNow), minute(timeNow), second(timeNow));

  // Print the formatted string to serial so we can see the time.
  Serial.print(buf);
  M5.Lcd.setCursor(100, 220);
  M5.Lcd.printf(buf);
}

}  // namespace

void setup() {
  Serial.begin(9600);
  //pinMode(DS1302_GND_PIN, OUTPUT);
  //pinMode(DS1302_VCC_PIN, OUTPUT);  

  // Enable RTC clock vcc pin and gnd pin
  //digitalWrite(DS1302_VCC_PIN, HIGH);
  //digitalWrite(DS1302_GND_PIN, LOW);
  
  if (enable_rtc_time_set) { 
    // Initialize a new chip by turning off write protection and clearing the
    // clock halt flag. These methods needn't always be called. See the DS1302
    // datasheet for details.
    rtc.writeProtect(false);
    rtc.halt(false);
  
    // Make a new time object to set the date and time.
    // Sunday, September 22, 2013 at 01:38:50.
    Time t(2018, 11, 9, 0, 19, 15, Time::kThursday);
  
    // Set the time and date on the chip.
     rtc.time(t);
  }
  // Retrieve the time and date from RTC
  Time t = rtc.time();
  // Set arduino internal clock the time and date retrieved from RTC
  setTime(t.hr,t.min,t.sec,t.date,t.mon,t.yr);
  // Disable the power of RTC clock
  //digitalWrite(DS1302_VCC_PIN, LOW);
  //digitalWrite(DS1302_GND_PIN, LOW);
  // initialize the M5Stack object
  M5.begin();
  M5.startupLogo();
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(1);

}

// Loop and print the time every second.

void loop() {
  M5.lcd.setBrightness(100);
  M5.Lcd.fillScreen(BLACK);
  printTime();
  sensorValue = analogRead(sensorPin);
  voltageValue = get_voltage_meas(sensorPin);
  Serial.print(" Analog Value: ");
  Serial.print(sensorValue);
  Serial.print(" Voltage Value: ");
  Serial.println(voltageValue);
  delay(5000);
  
  
}

float get_voltage_meas(int port){
  int rawvalue = 0;
  for (int i=0; i < precision_val; i++){
    rawvalue = rawvalue +  analogRead(port);
  }
  rawvalue = rawvalue / precision_val;
  float voltage_val = ((rawvalue / 1023.0)* V_REF * 5);
  voltage_val = voltage_val /1000.00; 
  return voltage_val;
}
