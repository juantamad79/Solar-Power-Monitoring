// Example sketch for interfacing with the DS1302 timekeeping chip.
//
// Copyright (c) 2009, Matt Sparks
// All rights reserved.
//
// http://quadpoint.org/projects/arduino-ds1302
#include <stdio.h>
#include <DS1302.h>
#include <TimeLib.h>
#include <SPI.h>
#include <SD.h>

namespace {

// Set the appropriate digital I/O pin connections. These are the pin
// assignments for the Arduino as well for as the DS1302 chip. See the DS1302
// datasheet:
//
//   http://datasheets.maximintegrated.com/en/ds/DS1302.pdf
Sd2Card card;
SdVolume volume;
SdFile root;
const int kCePin   = 7;                    // Chip Enable
const int kIoPin   = 18;                    // Input/Output
const int kSclkPin = 19;                    // Serial Clock
#define DS1302_GND_PIN 6                    // Gnd Pin for RTC
#define DS1302_VCC_PIN 5                    // VCC Pin for RTC
String datalog_file = "";                   // Variable for data log file to use in saving data to SD card
const byte chipSelect = 4;                  // Chip select pin for SD card
const int enable_data_logging = 1;          // enable this if you want to log data to SD card
const int enable_rtc_time_set = 0;          // enable this if you want to set new time/date in RTC
int sensorPin = A0;                         // select the input pin for the voltage measurement signal
int sensorValue = 0;                        // variable to store the value coming from the sensor
float voltageValue = 0;                     // Initial value for voltage measurement
#define V_REF 5090                          // Reference value 
#define precision_val 16                    // Precision value for getting voltage from analog input
String Serial_log = "";                     // String variable for logging data to SD card


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
  Serial_log = buf;
}

}  // namespace

void setup() {
  char buf(50);
  Serial.begin(9600);
  pinMode(DS1302_GND_PIN, OUTPUT);
  pinMode(DS1302_VCC_PIN, OUTPUT);  

  // Enable RTC clock vcc pin and gnd pin
  digitalWrite(DS1302_VCC_PIN, HIGH);
  digitalWrite(DS1302_GND_PIN, LOW);
  
  if (enable_rtc_time_set) { 
    // Initialize a new chip by turning off write protection and clearing the
    // clock halt flag. These methods needn't always be called. See the DS1302
    // datasheet for details.
    rtc.writeProtect(false);
    rtc.halt(false);
  
    // Make a new time object to set the date and time.
    // Sunday, September 22, 2013 at 01:38:50.
    Time t(2018, 11, 6, 2, 53, 10, Time::kTuesday);
  
    // Set the time and date on the chip.
     rtc.time(t);
  }
  // Retrieve the time and date from RTC
  Time t = rtc.time();
  // Set arduino internal clock the time and date retrieved from RTC
  setTime(t.hr,t.min,t.sec,t.date,t.mon,t.yr);
  // Disable the power of RTC clock
  digitalWrite(DS1302_VCC_PIN, LOW);
  digitalWrite(DS1302_GND_PIN, LOW);
  time_t timeNow = now();
  datalog_file.concat(year(timeNow));
  datalog_file.concat("");
  if (month(timeNow)<10){
   datalog_file.concat("0");
  }
  datalog_file.concat(month(timeNow));
  if (day(timeNow)<10){
   datalog_file.concat("0");
  }
  datalog_file.concat(day(timeNow));
  datalog_file.concat(".csv");
             
  Serial.print("Initializing SD card..."); 
  if (card.init(SPI_HALF_SPEED, chipSelect)) {
    Serial.println("Wiring is correct and a card is present.");
    Serial.print("\nCard type: ");
    switch (card.type()) {
      case SD_CARD_TYPE_SD1:
        Serial.print("SD1");
        break;
      case SD_CARD_TYPE_SD2:
        Serial.print("SD2");
        break;
      case SD_CARD_TYPE_SDHC:
        Serial.print("SDHC");
        break;
      default:
        Serial.println("Unknown");
    }
    if (!volume.init(card)) {
      Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
      return;
    }      
    uint32_t volumesize;
    Serial.print("\nVolume type is FAT");
    Serial.println(volume.fatType(), DEC);
    volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
    volumesize *= volume.clusterCount();       // we'll have a lot of clusters
    volumesize *= 512;                            // SD card blocks are always 512 bytes
    Serial.print("Volume size (Kbytes): ");
    volumesize /= 1024;
    Serial.println(volumesize);
    Serial.println("\nFiles found on the card (name, date and size in bytes): ");
    root.openRoot(volume);
    
      // list all files in the card with date and size
    root.ls(LS_R | LS_DATE | LS_SIZE);    
  }

  if (!SD.begin(chipSelect)) {
    Serial.println("FAILED.");
    // don't do anything more:
    return;
  }
  Serial.println("PASSED.");    
  delay(500);
  Serial.print("Initializing Log File..."); 
  Serial.print("File: ");
  Serial.print(datalog_file);   
  Serial.println("");          
}

// Loop and print the time every second.

void loop() {
  printTime();
  sensorValue = analogRead(sensorPin);
  voltageValue = get_voltage_meas(sensorPin);
  Serial.print(" Analog Value: ");
  Serial.print(sensorValue);
  Serial.print(" Voltage Value: ");
  Serial.println(voltageValue);
  Serial_log = Serial_log + "," + voltageValue;
  if (enable_data_logging == 1){
    File dataFile = SD.open(datalog_file, FILE_WRITE);
    if (dataFile) {
      dataFile.println(Serial_log);    
      
      dataFile.close(); 
    }
  }          
  delay(600000);
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
