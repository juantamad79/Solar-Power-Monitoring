
#include <Time.h>         //http://www.arduino.cc/playground/Code/Time  
#include <Wire.h>         //http://arduino.cc/en/Reference/Wire (included with Arduino IDE)
#include <SPI.h>
#include <SD.h>

Sd2Card card;
SdVolume volume;
SdFile root;
String datalog_file = "sample.log";
const byte chipSelect = 4;

void setup() {
  // put your setup code here, to run once:
    Serial.begin(9600);
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
}

void loop() {
  // put your main code here, to run repeatedly:
     File dataFile = SD.open(datalog_file, FILE_WRITE);
      if (dataFile) {
        dataFile.println("Testing");    
        dataFile.close(); 
      }
   delay(5000);
}
