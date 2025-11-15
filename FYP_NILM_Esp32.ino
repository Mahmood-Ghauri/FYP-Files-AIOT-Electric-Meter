#include <Wire.h>
#include <RTClib.h>
RTC_DS3231 rtc; // Create an instance of the RTC_DS3231 class
String simp_time = "NULL";
String date = "NULL";
int unixtime;

#include <TFT_eSPI.h> // Include the TFT_eSPI library
#include <SPI.h>
TFT_eSPI tft = TFT_eSPI();  // Create TFT object
unsigned long currentMillis, previousMillis = 0;  // Stores the last time display was updated
const long interval = 3000; 

// Variables to store received data of sensor
float voltage = 1;
float current = 1;
float app_power = 1;
float react_power = 1;
float act_power = 1;
float power_fact = 1;
// Variables to store ON/OFF cycle of appliances predicted by raspberry pi
String I_cycle = "ON";
String V_cycle = "ON";
String F_cycle = "ON";
String O_cycle = "ON";
// Variables to store active power of appliances calculated after scaling the predicted values by raspberry pi (0-1)
float I_act_power = 1;
float V_act_power = 1;
float F_act_power = 1;
float O_act_power = 1;
// Variables to store active power of appliances predicted by raspberry pi (0-1)
float D1_power = 1;
float D2_power = 1;
float D3_power = 1;
float D4_power = 1;
String receivedData;
String receivedData_2;

void setup()
{
  // Initialize Serial ports
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);
  Serial.println("Waiting for data...");

  Wire.begin();
  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  unsigned long unixTime = 1758445398;  // Example Unix timestamp
  rtc.adjust(DateTime(unixTime));

  tft.init();
  tft.setRotation(2); // Landscape

  delay(5000);
}

void loop()
{
  // Check if data is available on Serial (IRON)
  while (Serial.available() > 0)
  {
    receivedData = Serial.readStringUntil('\n'); // Read the data line
    // Calling Function to calculate values of iron
    raspberryData(receivedData);
  }

  while (Serial2.available() > 0) 
  {
    receivedData_2 = Serial2.readStringUntil('\n'); // Read the data line
    // Calling Function to calculate values of vacuum cleaner
    sensorData(receivedData_2); 
  }

  // Get RTC Time
  DateTime now = rtc.now();
  unixtime = now.unixtime();
  date = String(now.year()) + "-" + String(now.month()) + "-" + String(now.day());
  simp_time = String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second());

  Serial.print(unixtime);
  Serial.print(", ");
  Serial.print(act_power);
  Serial.println();

  currentMillis = millis();
  if(currentMillis - previousMillis >= interval)
  {
    tft_display();
    previousMillis = currentMillis;
  }

  delay(10); // Delay to avoid flooding with data
}

void tft_display()
{
  // Fill entire screen to avoid any grainy corners
  tft.fillScreen(TFT_LIGHTGREY);  // Light grey background

  // Medium bold title using Font 4 (fits nicely)
  tft.setTextFont(4); // Bold and medium
  tft.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  tft.setCursor(10, 10); // Adjust X for left margin
  tft.println("FYP AIOT METER");  // Shortened title for clean fit

  // Appliance statuses (left aligned)
  tft.setTextFont(2);
  tft.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  tft.setCursor(10, 50);
  tft.println("Fridge : " + String(F_cycle));

  tft.setTextFont(2);
  tft.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  tft.setCursor(10, 70);
  tft.println("Power  : " + String(F_act_power) + "W");


  tft.setTextFont(2);
  tft.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  tft.setCursor(10, 95);
  tft.println("Oven   : " + String(O_cycle));

  tft.setTextFont(2);
  tft.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  tft.setCursor(10, 115);
  tft.println("Power  : " + String(O_act_power) + "W");


  tft.setTextFont(2);
  tft.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  tft.setCursor(10, 140);
  tft.println("Iron   : " + String(I_cycle));

  tft.setTextFont(2);
  tft.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  tft.setCursor(10, 160);
  tft.println("Power  : " + String(I_act_power) + "W");
  

  tft.setTextFont(2);
  tft.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  tft.setCursor(10, 185);
  tft.println("Vacuum  : " + String(V_cycle));

  tft.setTextFont(2);
  tft.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  tft.setCursor(10, 205);
  tft.println("Power   : " + String(V_act_power) + "W");
}

// Function to sensor data
void sensorData(String data)
{
  int voltageIndex = data.indexOf("Voltage:");
  int currentIndex = data.indexOf("Current:");
  int power_factIndex = data.indexOf("Power Factor:");
  int app_powerIndex = data.indexOf("Apparent Power:");
 
  if (voltageIndex != -1)
  {
    voltage = data.substring(voltageIndex + 8, data.indexOf("V", voltageIndex)).toFloat();
  }

  if (currentIndex != -1)
  {
    current = data.substring(currentIndex + 8, data.indexOf("A", currentIndex)).toFloat();
  }

  if (app_powerIndex != -1)
  {
    app_power = data.substring(app_powerIndex + 15, data.indexOf("VA", app_powerIndex)).toFloat();
  }

  if (power_factIndex != -1)
  {
    power_fact = data.substring(power_factIndex + 13, data.indexOf("\n", power_factIndex)).toFloat();
    act_power = app_power * power_fact;
    react_power = sqrt(pow(app_power,2) - pow(act_power,2));
  }
}

// Function to Iron data
void raspberryData(String data)
{
  int D1_cycle_Index = data.indexOf("Iron:");
  int D2_cycle_Index = data.indexOf("Vacuum:");
  int D3_cycle_Index = data.indexOf("Fridge:");
  int D4_cycle_Index = data.indexOf("Oven:");
  int powerIndex = data.indexOf("Power:");
 
  if (D1_cycle_Index != -1 && powerIndex != -1)
  {
    I_cycle = data.substring(D1_cycle_Index + 10, data.indexOf(",", D1_cycle_Index));
    I_act_power = data.substring(powerIndex + 19, data.indexOf("W", powerIndex)).toFloat();
  }

  if (D2_cycle_Index != -1 && powerIndex != -1)
  {
    V_cycle = data.substring(D2_cycle_Index + 10, data.indexOf(",", D2_cycle_Index));
    V_act_power = data.substring(powerIndex + 19, data.indexOf("W", powerIndex)).toFloat();
  }

  if (D3_cycle_Index != -1 && powerIndex != -1)
  {
    F_cycle = data.substring(D3_cycle_Index + 10, data.indexOf(",", D3_cycle_Index));
    F_act_power = data.substring(powerIndex + 19, data.indexOf("W", powerIndex)).toFloat();
  }

  if (D4_cycle_Index != -1 && powerIndex != -1)
  {
    O_cycle = data.substring(D4_cycle_Index + 10, data.indexOf(",", D4_cycle_Index));
    O_act_power = data.substring(powerIndex + 19, data.indexOf("W", powerIndex)).toFloat();
  }
}