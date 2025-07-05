#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <FirebaseESP32.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include "TickTwo.h"
#include <HTTPClient.h>
#include <ZMPT101B.h>
#include <DHT.h>

//monthly bill analisis
//





FirebaseConfig config;
FirebaseAuth auth;

#define DHTPIN 27       
#define DHTTYPE DHT11 

// Create DHT object
DHT dht(DHTPIN, DHTTYPE);

  float Humidity;
  float Temperature;

// Calibration offsets DHT11
const float tempOffset = -3.0;    
const float humOffset = -7.0; 

#define WIFI_SSID "XOR"  // Wifi Username
#define WIFI_PASSWORD "012345600"   // Password
//String GOOGLE_SCRIPT_ID = "***";


const char* host = "script.google.com";
const int httpsPort = 443;

LiquidCrystal_I2C lcd(0x27, 16, 2);


FirebaseData firebaseData;

#define SENSITIVITY 490.0f
ZMPT101B voltageSensor(34, 50.0);

// Variables for energy calculation
float kWh = 0.0;
float lastKWhSent = 0.0;  // Track the last kWh sent for billing
float lastKWhSentfirebase=0.0;
unsigned long lastMillis = 0; // Fix typo: Change "mills()" to "0"
unsigned long previousMillis = 0;
const long intervel=5000;
bool firstRun = true; // Variable to track if it's the first run
float bill = 0.0;  


// EEPROM addresses for each variable
const int addrVrms = 0;
const int addrIrms = 12;
const int addrPower = 4;
const int addrKWh = 8;
const int addBill = 16;


// Billing rates
const float rateBlock1 = 12.00;
const float rateBlock2 = 30.00;
const float rateBlock3 = 38.00;
const float rateBlock4 = 41.00;
const float rateBlock5 = 59.00;
const float rateBlock6 = 89.00;

// Function prototypes
void sendEnergyDataToFirebase();
void readEnergyDataFromEEPROM();
void saveEnergyDataToEEPROM();

TickTwo timer1(sendEnergyDataToFirebase,5000);

void setup() {

  Serial.begin(115200);
  delay(1000);

  // wifi Connecting
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  //Firebase
  config.host="https://flutterfirebase-dc911-default-rtdb.asia-southeast1.firebasedatabase.app/";
  config.api_key="AIzaSyDo3eZIkYaMOoT1cVp5Il6CHsxIIb5xQ_0";

  auth.user.email = "riththikkansairam@gmail.com"; // Replace with your email used for Firebase
  auth.user.password = "roomsync"; // Replace with your Firebase password
  
  // Initialize the LCD
  lcd.init();
  lcd.backlight();
  //Initialize the DHT11
  dht.begin();

  // Setup Firebase
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Serial.print("sari sari connecting\n");

   timer1.start();


  // Initialize EEPROM with the size of the data to be stored
    EEPROM.begin(32); // Only one float value, kWh


  // Read the stored energy data from EEPROM
  readEnergyDataFromEEPROM();


 

  voltageSensor.setSensitivity(SENSITIVITY);
}

void loop() {
   timer1.update();
}

void sendEnergyDataToFirebase() {

  float voltage = voltageSensor.getRmsVoltage();
  float Irms=calculateIrms(33,2000);
  float Power = voltage*Irms;
  float Temperature = calculateTemp();
  float Humidity = calculateHum();

  if (voltage < 180 ) {
    voltage=0;
    Irms=0;
    Power=0;
  }

  // Calculate energy consumed in kWh
  unsigned long currentMillis = millis();
  kWh += Power * (currentMillis - lastMillis) / 3600000000.0;
  lastMillis = currentMillis;

  // If the initial kWh is zero, set power to zero
  if (kWh == 0) {
    Power = 0.0;
  }
  
  // Print data to Serial for debugging
  float usage = kWh-lastKWhSentfirebase;
    float bill = calculateBill(usage);
  Serial.printf("Vrms: %.4fV\tIrms: %.4fA\tPower: %.4fW\tkWh: %.5fkWh\tBill: %.2fRs\nTemperture: %.2f\tHumidity: %.2f\n",
                voltage, Irms, Power, kWh, bill, Temperature, Humidity);

  // Save the latest values to EEPROM
 saveEnergyDataToEEPROM();
 
 if (!Firebase.setFloat(firebaseData, "conditions/Voltage", voltage)) {
  Serial.println("Failed to write voltage: " + firebaseData.errorReason());
}
Firebase.setFloat(firebaseData, "conditions/KWh", kWh);

    
    Firebase.setFloat(firebaseData, "EnergyData/Voltage", voltage);
    Firebase.setFloat(firebaseData, "EnergyData/Current", Irms);
    Firebase.setFloat(firebaseData, "EnergyData/Power", Power);
    Firebase.setFloat(firebaseData, "EnergyData/KWh", kWh);
    Firebase.setFloat(firebaseData, "EnergyData/temp", Temperature);
    Firebase.setFloat(firebaseData, "EnergyData/humidity", Humidity);
    
    // Firebase update bill in every 5sec
    Firebase.setFloat(firebaseData, "EnergyData/Bill", bill);
    
    //firebase rest the bill amount in every 1min 
    if(currentMillis-previousMillis>=60000){
      previousMillis=currentMillis;
      bill=0.0;
      lastKWhSentfirebase=kWh;
    }
  
  // Update the LCD with the new values
 

// Display Voltage and Irms
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Vrms: ");
  lcd.print(voltage, 2);
  lcd.print(" V");
  

  
  lcd.setCursor(0, 1);
  lcd.print("Irms: ");
  lcd.print(Irms, 4);
  lcd.print(" A");
  delay(3000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Power: ");
  lcd.print(Power, 4);
  lcd.print(" W");
  

  
  lcd.setCursor(0, 1);
  lcd.print("kWh: ");
  lcd.print(kWh, 5);
  lcd.print(" kWh");
  delay(3000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Bill Amount: ");
  lcd.setCursor(0, 1);
  lcd.print(bill, 2);
  lcd.print(" rs");
  delay(3000);


  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(Temperature, 2);
  lcd.print("'c");
  
  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(Humidity, 2);
  //lcd.print("");
  delay(3000);

}




float calculateBill(float energyConsumed)
{
  float billAmount = 0.0;

  if (energyConsumed <= 30)
    billAmount = energyConsumed * rateBlock1;
  else if (energyConsumed <= 60)
    billAmount = 30 * rateBlock1 + (energyConsumed - 30) * rateBlock2;
  else if (energyConsumed <= 90)
    billAmount = 30 * rateBlock1 + 30 * rateBlock2 + (energyConsumed - 60) * rateBlock3;
  else if (energyConsumed <= 120)
    billAmount = 30 * rateBlock1 + 30 * rateBlock2 + 30 * rateBlock3 + (energyConsumed - 90) * rateBlock4;
  else if (energyConsumed <= 180)
    billAmount = 30 * rateBlock1 + 30 * rateBlock2 + 30 * rateBlock3 + 30 * rateBlock4 + (energyConsumed - 120) * rateBlock5;
  else
    billAmount = 30 * rateBlock1 + 30 * rateBlock2 + 30 * rateBlock3 + 30 * rateBlock4 + 60 * rateBlock5 + (energyConsumed - 180) * rateBlock6;

  return billAmount;
}




 float calculateTemp() {
  float rawTemp = dht.readTemperature();
  if (isnan(rawTemp)) {
    Serial.println("Failed to read from DHT Temperature sensor!");
    return 0.0;
  } else {
    return rawTemp + tempOffset;
  }
}

float calculateHum() {
  float rawHum = dht.readHumidity();
  if (isnan(rawHum)) {
    Serial.println("Failed to read from DHT Humidity sensor!");
    return 0.0;
  } else {
    return rawHum + humOffset;
  }
}







 
 
void readEnergyDataFromEEPROM()
{
  // Read the stored kWh value from EEPROM
  EEPROM.get(addrKWh, kWh);
  EEPROM.get(addBill,bill);
 
  // Check if the read value is a valid float. If not, initialize it to zero
  if (isnan(kWh))
  {
    kWh = 0.0;
    saveEnergyDataToEEPROM(); // Save initialized value to EEPROM
  }
  if (isnan(bill)){
    bill = 0;
    saveEnergyDataToEEPROM();
  }
}
 
void saveEnergyDataToEEPROM()
{
  // Write the current kWh value to EEPROM
  EEPROM.put(addrKWh, kWh);
  EEPROM.put(addBill,bill);
  // Commit changes to EEPROM
  EEPROM.commit();
}
//encode the sending values
String urlEncode(const String &value) {
  String encodedValue = "";
  char c;
  for (size_t i = 0; i < value.length(); i++) {
    c = value.charAt(i);
    if (isAlphaNumeric(c) || c == '-' || c == '.' || c == '_' || c == '~') {
      encodedValue += c;
    } else {
      encodedValue += "%" + String(c, HEX);
    }
  }
  return encodedValue;
}

float calculateIrms(float value, int sampleCount) {
  float mappedValue = 0;
  float sumSquareValue = 0;

  for (int i = 0; i < sampleCount; i++) {
    int sensorValue = analogRead(33);
    float voltage = sensorValue * (3.3 / 4095);
    float dcCurrent = (voltage - 2.38) / 0.22;
    mappedValue = dcCurrent;
    float squreValue = (pow(mappedValue, 2));
    sumSquareValue += squreValue;
    delay(1);
  }
  float result = sqrt(sumSquareValue / sampleCount)-0.088;
  if (result <= 0) {
    return 0;
  } else {
    return result;
  }
}
