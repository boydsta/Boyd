/**
 *This sketch created by Boyd Benton
 */

#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"'
#include "driver/adc.h" //adc library
#include "DHT.h"
#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#define DHTPIN 26
#define DHTTYPE DHT11
// Initialize the OLED display using Wire library
SSD1306 display(0x3c, 5, 4);
// Auth Token for the Blynk App.
// char auth[] = "0a5c5275307b4acfa0298f4064e61139";

char auth[] = "bb0aa5a30d8c450ba3c16d35ee10dbbb";
// Your WiFi credentials.
char ssid[] = "Sunrise_2.4GHz";
char pass[] = "earlytobed";
// char ssid[] = "ecoinnovation";
// char pass[] = "ecoinnovation";
// char ssid[] = "TinyHouse";
// char pass[] = "MyToilet";

int SensorPin1 = 39;   // select the input pin for PV Voltage
int SensorPin2 = 36;   // select the input pin for Batt Voltage
int Value1 = 0; //variable for reading analogue pin
int Value2 = 0; //variable for reading analogue pin
float PV_V = 0; //variable for analogure read of PV Voltage
float Batt_V = 0; //variable for analogure read of Battery Voltage
float humd = 0; //variable for analogure read of Humidity
float temp = 0; //variable for analogure read of Temperature

// Define the number of samples to keep track of. The higher the number, the
// more the readings will be smoothed, but the slower the output will respond to
// the input. Using a constant rather than a normal variable lets us use this
// value to determine the size of the readings array.
const int numReadings = 80;

int readingsPV_V[numReadings];      // the readings from the analog input
int readingsBatt_V[numReadings];  // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int totalPV_V = 0;                  // the running total
int totalBatt_V = 0;                 // the running total   
int averagePV_V = 0;                // the average
int averageBatt_V = 0;                // the average





DHT dht(DHTPIN, DHTTYPE);

BlynkTimer timer;
// This function sends Arduino's up time every second to Virtual Pin (5).
// In the app, Widget's reading frequency should be set to PUSH. This means
// that you define how often to send data to Blynk App.

void setup() 
   {
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);
  pinMode(SensorPin1, INPUT);
  pinMode(SensorPin2, INPUT);
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_24);
  display.setColor(WHITE);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_24);
  //DHT11
  Serial.println("DHT11 sensor!");
  //call begin to start sensor
  dht.begin();
  // initialize all the readings to 0: for smoothing out PV_Voltage.
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readingsPV_V[thisReading] = 0;
    readingsBatt_V[thisReading] = 0; 
  }
  
  // Setup a function to be called every second
  timer.setInterval(5000L, myTimerEvent);
  }

void loop() {
  
  //Value1 = analogRead(SensorPin1); //ADC Read for PV_V
  //Value2 = analogRead(SensorPin2); //ADC Read for Batt_V
  humd = dht.readHumidity();       // Read temperature as Celsius (the default)
  temp = dht.readTemperature();    // Check if any reads failed and exit early (to try again).
  if (isnan(humd) || isnan(temp)) {
    Serial.println("Failed to read from DHT sensor!");
    delay (10);
    return;
  }
 // subtract the last reading:
  totalPV_V = totalPV_V - readingsPV_V[readIndex];
  totalBatt_V = totalBatt_V - readingsBatt_V[readIndex];
  // read from the sensor:
  readingsPV_V[readIndex] = analogRead(SensorPin1); //ADC Read for PV.
  readingsBatt_V[readIndex] = analogRead(SensorPin2); //ADC Read for Batt_V.
  // add the reading to the total:
  totalBatt_V = totalBatt_V + readingsBatt_V[readIndex];
  totalPV_V = totalPV_V + readingsPV_V[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }

  // calculate the average:
  averagePV_V = totalPV_V / numReadings;
 averageBatt_V = totalBatt_V / numReadings;
  
   PV_V = map(averagePV_V, 0, 4095, 0, 3106.7); //calibration
   Batt_V = map(averageBatt_V, 0, 4095, 0, 3106.7); //calibration
   PV_V = PV_V /100; // set to 2 decimal places
   Batt_V =Batt_V /100;  // set to 2 decimal places

 UpdateDisplay(); //writes to the display.
   
  Blynk.run();
  timer.run(); // Initiates BlynkTimer
  
delay (10);
}

void myTimerEvent()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V21,humd);
  Blynk.virtualWrite(V20,temp);
  Blynk.virtualWrite(V2,PV_V);
  Blynk.virtualWrite(V3,Batt_V);
  Blynk.virtualWrite(V7,PV_V-Batt_V);
  Blynk.virtualWrite (V23, Batt_V);
  
 
 UpdatePrint(); // for diagnostics only
  }

  void UpdateDisplay()
{
  display.clear();
  display.drawString(64, 1, String(PV_V)+ " V-PV ");
  display.drawString(64, 20, String(Batt_V)+ " V-Bat");
  display.drawString(64, 40, String(temp)+ " C ");
  display.display();
}

  void UpdatePrint()
{
   Serial.print(Batt_V); // print value
  Serial.print(" ,"); // print value
  Serial.println(PV_V); // print value 
  Serial.print("Humidity: ");
  Serial.print(humd);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.println(" *C ");
}


