#include <WiFi.h>
#include "time.h"

// Network credentials
const char* ssid     = "SERAPHIX";
const char* password = "hyperpineapple0452";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 19800;
const int   daylightOffset_sec = 0;


int ct_resistance = 150;
int ct_ratio = 2500;
float red_line_current;
float blue_line_current;
float yellow_line_current;
float threshold = 10.0;
float threshold_upper = threshold + 0.5;
float threshold_lower = threshold = 0.5;


void printLocalTime() //Function to get date and time
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo);
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

float voltageToCurrent() //Convert voltage obtained from CT to Required current
{
  return (ct_ratio*value*1.3/(ct_resistance*4096)); // Dividing resistance after converting it back to analog values. Multiplying with CT's factor
}

void getCurrentValue()
{ 
  red_line_current = voltageToCurrent(analogRead(36)); //Connect Red line to GPIO36
  delayMicroseconds(6666); //120 Phase shift in 50 Hz corresponds to a 6.66 ms delay
  blue_line_current = voltageToCurrent(analogRead(39)); //Connect Blue line to GPIO39
  delayMicroseconds(6666);
  yellow_line_current = voltageToCurrent(analogRead(34)); //Connect Yellow line to GPIO34
  delayMicroseconds(6666);
}

void triggerLEDS() //Function to trigger LEDs
{ if(red_line_current || blue_line_current || yellow_line_current > threshold_upper) 
  {
    //function to trigger Red LED
  }
  else
  {
    if (red_line_current || blue_line_current || yellow_line_current > threshold)
    {
      //function to trigger Blue LED
    }
    else
    {
      //function to trigger Green LED
    }
  }

}

void setup()
{
  analogSetAttenuation(ADC_6db); //Seeting the attenuation to 6db, Setting the limit of voltage read to 1.3 Volts
  Serial.begin(115200);
  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println(" CONNECTED");
  //initalize and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
}

void loop()
{
  delay(1000);
  printLocalTime();
  getCurrentValue();
  triggerLEDS();

}
