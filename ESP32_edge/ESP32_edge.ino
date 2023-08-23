#include <WiFi.h>
#include "time.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SD.h>
#include <LoRa.h>
#include <edge_info.h>

// Network credentials
const char* ssid     = EDGE_SSID;
const char* password = EDGE_PASSWORD;

//NTP Server Configuration
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 19800; //IST +05:30  
const int   daylightOffset_sec = 0;

//OLED Screen Configuration
#define OLED_SDA 21
#define OLED_SCL 22 
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Change this to match SD card module's CS (Chip Select) pin
const int chipSelect = 2; 

//define the pins used by the transceiver module
#define ss 12
#define rst 14
#define dio0 39

// Declaring Variables
int ct_resistance = 150;
int ct_ratio = 2500;
float red_line_current, blue_line_current, yellow_line_current, current_rms;
float threshold = 10.0;
float threshold_upper = threshold + 0.5;
String dayOfWeek, date, exactTime, LoRa_transmitted_data;

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

float voltageToCurrent(int value) //Convert voltage obtained from CT to Required current
{
  return (ct_ratio*value*1.3/(ct_resistance*4096)); // Dividing resistance after converting it back to analog values. Multiplying with CT's factor
}

void getCurrentValue() //Function to get the value of current in each phase line
{ 
  float red_line_current_instant = 0;
  float blue_line_current_instant = 0;
  float yellow_line_current_instant = 0;
  float current_rms_instant;
  int count;
  

  for (count = 0; count <10; count++) 
  {
    display.clearDisplay(); //Clear Display
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.setTextSize(1);
    display.print("Threshold: "); //Display threshold value
    display.print(threshold);
    display.print(" A");

    red_line_current_instant = analogRead(36);//Connect Red line to GPIO36
    red_line_current_instant = voltageToCurrent(red_line_current_instant);
    display.setCursor(0,10);
    display.print("R: "); //Display Red Line current
    display.print(red_line_current_instant);
    display.print(" A");
    red_line_current += red_line_current_instant;
    

    blue_line_current_instant = analogRead(39); //Connect Blue line to GPIO39
    blue_line_current_instant = voltageToCurrent(blue_line_current_instant); 
    display.setCursor(0,20);
    display.print("B: "); //Display Blue Line current
    display.print(blue_line_current_instant);
    display.print(" A");
    blue_line_current += blue_line_current_instant;
    

    yellow_line_current_instant = analogRead(34); //Connect Yellow line to GPIO34
    yellow_line_current_instant = voltageToCurrent(yellow_line_current_instant);
    display.setCursor(0,30);
    display.print("Y: "); //Display Yellow Line current
    display.print(yellow_line_current_instant);
    display.print(" A");
    
    yellow_line_current += yellow_line_current_instant;

    current_rms_instant = sqrt((red_line_current_instant)*(red_line_current_instant) + (blue_line_current_instant)*(blue_line_current_instant) + (blue_line_current_instant)*(blue_line_current_instant)); // Calculate Average RMS Value
    display.setCursor(0,40);
    display.print("Final: ");
    display.print(current_rms_instant);
    display.print(" A");
    display.display();

    // localTime();
    delay(1000); // 6 second delay 
  }
  localTime();
  red_line_current = red_line_current/10;
  blue_line_current = blue_line_current/10;
  yellow_line_current = yellow_line_current/10;

  current_rms = sqrt((red_line_current)*(red_line_current) + (blue_line_current)*(blue_line_current) + (blue_line_current)*(blue_line_current)); // Calculate Average RMS Value
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



void transmittData(int region, float value_1, float value_2, float value_3, float value_4, float value_5) //1 threshold, 3 phase values, 1 main current
{ 
  Serial.print("Sending packet: ");
  LoRa_transmitted_data = String(region) + ',' + String(value_1) + ',' + String(value_2) + ',' + String(value_3) + ',' + String(value_4) + ',' + String(value_5);
  //Send LoRa packet to receiver
  // LoRa.beginPacket();
  // LoRa.print(LoRa_transmitted_data);
  // LoRa.endPacket();
  Serial.println(LoRa_transmitted_data);

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
  localTime();

  // SD Initialization
  if (!SD.begin(2)) {
    Serial.println("Card failed, or not present");
  } else {
    Serial.println("Card initialized.");
  }

  //OLED Initialization
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS, false, false)) {
    Serial.println("SSD1306 allocation failed"); // Don't proceed, loop forever
  }

  // Setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);
  // replace the LoRa.begin argument with your location's frequency 
  // 433E6 for Asia, 866E6 for Europe, 915E6 for North America
  while (!LoRa.begin(433E6)) { 
    Serial.println(".");
    delay(500);
  }
  Add a sync word to match the receiver for encryption purposes if required
  Serial.println("LoRa Initializing OK!");

  //Initialize LEDs 
  pinMode(14, OUTPUT);    // Red LED
  pinMode(4, OUTPUT);     // Blue LED
  pinMode(26, OUTPUT);    // Green LED
  pinMode(34, INPUT);
  pinMode(36, INPUT);
  pinMode(39, INPUT);
}

void loop()
{
  // delay(1000);
  Serial.println(dayOfWeek);
  Serial.println(date);
  Serial.println(exactTime);
  getCurrentValue();
  triggerLEDS();
  transmittData(1, threshold,red_line_current, blue_line_current, yellow_line_current, current_rms); // Sent in the format of Unit, Threshold, Red, Blue, Yellow, RMS
}
