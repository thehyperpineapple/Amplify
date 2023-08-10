#include <WiFi.h>
#include "time.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SD.h>

// Network credentials
const char* ssid     = "SERAPHIX";
const char* password = "hyperpineapple0452";

//NTP Server Configuration
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 19800; //IST +05:30  
const int   daylightOffset_sec = 0;

//OLED Screen Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Change this to match your SD card module's CS (Chip Select) pin
const int chipSelect = 2; 

// Declaring Variables
int ct_resistance = 150;
int ct_ratio = 2500;
float red_line_current;
float blue_line_current;
float yellow_line_current;
float current_rms;
float threshold = 10.0;
float threshold_upper = threshold + 0.5;
float threshold_lower = threshold - 0.5;
String dayOfWeek;
String date;
String exactTime;

void printLocalTime() // Function to get date and time
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
  dayOfWeek = dayOfWeek_array;
  date = date_array;
  exactTime = exactTime_array;
  Serial.println(dayOfWeek);
  Serial.println(date);
  Serial.println(exactTime);
}

float voltageToCurrent(int value) //Convert voltage obtained from CT to Required current
{
  return (ct_ratio*value*1.3/(ct_resistance*4096)); // Dividing resistance after converting it back to analog values. Multiplying with CT's factor
}

float getCurrentValue() //Function to get the value of current in each phase line
{ 
  float red_line_current_instant = 0;
  float blue_line_current_instant = 0;
  float yellow_line_current_instant = 0;
  int count;
  display.clearDisplay(); //Initialize Display
  display.setCursor(0,0);
  display.setTextSize(1);
  display.println("Threshold: "); //Dsiplay threshold value
  display.print(threshold);
  display.print(" A");

  for (count = 0; count <10; count++) 
  {
    red_line_current_instant = analogRead(36);//Connect Red line to GPIO36
    red_line_current_instant = voltageToCurrent(red_line_current_instant);
    display.println("R: "); //Display Red Line current
    display.print(red_line_current_instant);
    display.print(" A");
    red_line_current += red_line_current_instant;
    

    blue_line_current_instant = analogRead(39); //Connect Blue line to GPIO39
    blue_line_current_instant = voltageToCurrent(blue_line_current_instant); 
    display.println("B: "); //Display Blue Line current
    display.print(blue_line_current_instant);
    display.print(" A");
    blue_line_current += blue_line_current_instant;
    

    yellow_line_current_instant = analogRead(34); //Connect Yellow line to GPIO34
    yellow_line_current_instant = voltageToCurrent(yellow_line_current_instant);
    display.println("Y: "); //Display Yellow Line current
    display.print(yellow_line_current_instant);
    display.print(" A");
    yellow_line_current += yellow_line_current_instant;
    
    delay(6000); // 6 second delay 
  }
  red_line_current = red_line_current/10;
  blue_line_current = blue_line_current/10;
  yellow_line_current = yellow_line_current/10;

  current_rms = sqrt((red_line_current)*(red_line_current) + (blue_line_current)*(blue_line_current) + (blue_line_current)*(blue_line_current));

  return red_line_current, blue_line_current, yellow_line_current, current_rms;
}

void triggerLEDS() //Function to trigger LEDs. Red - GPIO14, Green - GPIO26, Blue - GPIO4
{ 
  if(current_rms > threshold_upper) 
  {
    digitalWrite(14, HIGH);    // Turn on Red LED
    digitalWrite(4, LOW);     // Turn off Blue LED
    digitalWrite(26, LOW);    // Turn off Green LED
  }
  else
  {
    if (current_rms > threshold)
    {
      digitalWrite(14, LOW);    // Turn off Red LED
      digitalWrite(4, HIGH);     // Turn on Blue LED
      digitalWrite(26, LOW);    // Turn off Green LED
    }
    else
    {
      digitalWrite(14, LOW);    // Turn off Red LED
      digitalWrite(4, LOW);     // Turn off Blue LED
      digitalWrite(26, HIGH);    // Turn on Green LED
    }
  }
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
    dataFile.print("R B Y X");
    dataFile.close();
    Serial.println("New values added.");
  } else {
    Serial.println("Error opening file.");
  }
}

void setup()
{
  analogSetAttenuation(ADC_6db); //Setting the attenuation to 6db, Setting the limit of voltage read to 1.3 Volts
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
  printLocalTime();
  if (!SD.begin(2)) {
    Serial.println("Card failed, or not present");
  } else {
    Serial.println("Card initialized.");
  }
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("SSD1306 allocation failed");
    for(;;); // Don't proceed, loop forever
  }
}

void loop()
{
  delay(1000);
  printLocalTime();
  getCurrentValue();
  triggerLEDS();
  writeToSD();
}
