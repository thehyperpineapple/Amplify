#include <WiFi.h>
#include "time.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SD.h>
#include <SPI.h>
#include <LoRa.h>

// Network credentials
const char* ssid     = "SERAPHIX";
const char* password = "hyperpineapple0452";

//NTP Server Configuration
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 19800; //IST +05:30  
const int   daylightOffset_sec = 0;

String unit, threshold, red_line_current, blue_line_current, yellow_line_current, current_rms, current_rms_1, current_rms_2, current_rms_3 = "";

//OLED Screen Configuration
#define OLED_SDA 21
#define OLED_SCL 22 
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Change this to match your SD card module's CS (Chip Select) pin
const int chipSelect = 2; 

//define the pins used by the transceiver module
#define ss 12
#define rst 14
#define dio0 39

String dayOfWeek, date, exactTime, LoRaData, to_cloud_data;

void localTime() // Function to get date and time
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  char dayOfWeek_array[20]; // Array to store the day of the week
  char date_array[30];      // Array to store the formatted date
  char exactTime_array[20];      // Array to store the formatted time

  strftime(dayOfWeek_array, sizeof(dayOfWeek_array), "%A", &timeinfo);
  strftime(date_array, sizeof(date_array), "%d-%B-%Y", &timeinfo);
  strftime(exactTime_array, sizeof(exactTime_array), "%H:%M:%S", &timeinfo);
  dayOfWeek = String(dayOfWeek_array);
  date = String(date_array);
  exactTime = String(exactTime_array);
  
}

void receiveData()
{
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    while (LoRa.available()) {
      LoRaData = LoRa.readString();
      Serial.print(LoRaData);
    }
  }
  //print RSSI of packet
  int rssi = LoRa.packetRssi();
  Serial.print(" with RSSI ");    
  Serial.println(rssi);
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("Master Node");
  display.setCursor(0,10);
  display.print("RSSI: ");
  display.print(rssi);
  

  // Seperate Data
  int commaPos = -1;
  for (int i = 0; i < 6; i++) {
    commaPos = LoRadata.indexOf(',');
    if (commaPos != -1) {
      if (i == 0) {
        unit = LoRadata.substring(0, commaPos);
      } else if (i == 1) {
        threshold = LoRadata.substring(0, commaPos);
      } else if (i == 2) {
        red_line_current = LoRadata.substring(0, commaPos);
      } else if (i == 3) {
        blue_line_current = LoRadata.substring(0, commaPos);
      } else if (i == 4) {
        yellow_line_current = LoRadata.substring(0, commaPos);
      } else if (i == 5) {
        current_rms = LoRadata.substring(0, commaPos);
      }
      LoRadata = LoRadata.substring(commaPos + 1);
    } else {
      current_rms = data;
    }
    if (unit == "1") {
      current_rms_1 = current_rms;
    } else if (unit =="2") {
      current_rms_2 = current_rms;
    } else if (unit =="3") {
      current_rms_3 = current_rms;
    }
  }
  display.setCursor(0,30);
  display.print("Unit 1: " + current_rms_1 " A");
  display.setCursor(0,40);
  display.print("Unit 2: " + current_rms_2 " A");
  display.setCursor(0,50);
  display.print("Unit 3: " + current_rms_3 " A");
  display.display();
}

void writeToSD() // Function to read and write SD card
{
    if (!SD.exists("data.txt")) {
    Serial.println("File does not exist, creating new file.");
    
    // Open the file in write mode to create it
    File dataFile = SD.open("data.txt", FILE_WRITE);
    
    if (dataFile) {
      dataFile.close();
      Serial.println("New file created.");
    } else {
      Serial.println("Error creating file.");
    }
  }

  Serial.println("Values do not exist, adding new values.");
  // Open the file in append mode
  File dataFile = SD.open("data.txt", FILE_WRITE);

  if (dataFile) {
    dataFile.print(LoRaData);
    dataFile.close();
    Serial.println("New values added.");
  } else {
    Serial.println("Error opening file.");
  }
}

void sendToCloud()
{
  to_cloud_data = dayOfWeek + ',' date
}

void setup() {
  Serial.begin(115200);

  //Connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println("CONNECTED");

  //initalize and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  localTime();
  LoRa.setPins(ss, rst, dio0);

  while (!LoRa.begin(433E6)) {
    Serial.println(".");
    delay(500);
  }
   // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  Serial.println("LoRa Initializing OK!");

}

void loop() {
  localTime();
  receiveData();
  writeToSD();
}
