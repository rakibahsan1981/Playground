#define BLYNK_TEMPLATE_ID "TMPL6Z9-S99Lf"
#define BLYNK_TEMPLATE_NAME "IoT Energy Meter"
#define BLYNK_PRINT Serial

#include "EmonLib.h"
#include <EEPROM.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <ESP_Google_Sheet_Client.h>
#include <time.h>

// Constants for calibration
const float vCalibration = 41.5;
const float currCalibration = 0.15;

// Blynk and WiFi credentials
const char auth[] = "yrIuEKsZbFSr7kHNXJEPcojSkbW6mk3Z";
const char ssid[] = "Honeypot";
const char pass[] = "01552368187";

// Google Sheets credentials
#define PROJECT_ID "vernal-buffer-466805-v9"
#define CLIENT_EMAIL "srvdatalogger@vernal-buffer-466805-v9.iam.gserviceaccount.com"
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----\nMIIEvAIBADANBgkqhkiG9w0BAQEFAASCBKYwggSiAgEAAoIBAQDQPiKYcNDTg6jK\n10S5vFiXy7m5XF2t+kavZyLSP0XJtwKikAzONmF35OA1ICzCZEWJDG0mbM9cPK7a\nQJyBYmhiaWupXOQOLZEQ9y3AyAwRE6JbN6xMSHnJA/X8S4EzORv7C2mH8TOCE0ih\n/4hcgFSxoaLcPFPJxTYsIXmqPRcY9jnc/dT+WABNwkLkp2nIViqfxgAWmei35mw4\nmX0UwXCnKVksvSkggWXTh/V+pfD8K0lTEz6Ht1R7ZeovLbmpP5+g+QlJra+77dAC\nR/g7BwIXKcWqGsHOj4XtLNnePRtFn2baBcTm0KHH+gg3t2BAniFfwGac1YI/T8Rj\nGYWrXJkzAgMBAAECggEAA6K0vOo/Tb9fMNy4QkaJBMFq8ShX1Fk/EayLHZY63tjX\nRBpKtvDucQbpvn5czwSsIsaOQ0ekCSK1ahxgVHFGNu2gkmOLnGRsQ2+6o0J61MjT\nIqTirlmXkrcDJ4/z5f6JwATE0g7tIh4PEbuh2FI/PZ3/og5a53YOH5AdYnz64cB7\n/l1/zcDoaoudp+nMTXaXTIbVNKLH7t/Gf4cD3QPwrVMYMcCGcCc5vHEvQe0TPvSy\nHiCFAUjm4LHSFG1rDxP5c5uu2vDrj0cP0qnH3LRhKAtF+4QVywAm3sAdpi3GvpXJ\npIVhYSEOtqZ4KxQldWQqfIKDqSioXgT3VLYKO9wRoQKBgQDx2PP4Utsfkl9X/BwK\nNd3b6bTrh5FBMxOXovs2DiWjCahnhmTjfY2CuJ8/mSmejfUFwj1DhQNvNRhwe/SP\nNaymdWTGOAHtrALcGjqZ+8mpxa/SEOy37MOplEuGyjkU7VjLBbJr8/mFMnEf3rIn\nCYHjTUWEpmZxjBBqzJgoSpbX4QKBgQDcbcJNAdfBHritoiZqqd/9w6X3jbtMWIzd\nUTluSTdfsqlJw7fOJ6IiM1okI9XuHKIXhyWai9dSls5T3oSlI23cz5ckPT1EDN68\nesR/JAAupPxZ0l5V3HBXWnygBHHmavOC9krK+yuSoxFR6BvQMk8r6N0RZRQSAMZN\nBMPnTRcDkwKBgG5EphAt7vy5J0GdkEdTgiF3sGbHPAJHQIKd6/4ceyqYB7GUBEH5\nB+F3PhyoP+KsBPCoPHihmAxHYCpBSiNVrK+EzPrXpIFyMSebyeZVeAAxQ1X7I4NE\nr32NQNZnM5mIEjkXEt/HsJf7hLqsNfw4iIFTxQOTPxg8bWvUdve/8oDhAoGABXjo\nGPdRvuafWRgscftzpx4jUakAHF4aaSgqD9XbfRG3aDBmsMSFHuo9c6Y8GmQ1lfXx\n9gtAtjkuOCDPFDZz9MZmjyzCB512LiGyHSHsqzvHVAeH9gO5+kEx1NsnZLwE/ZxV\nsQJZS5Omy2zCvAFzu7hVPKhQvj2srp7OqkBcsOcCgYByipfUk0ECaWKDjFI3Itx+\n3teQUQuJb9SUAlK2Xqy3G8St/YPybmzrDP4iFiQ6loi4D1feOu7+mnfuSx6wqwVF\noubxWKhlG0AvBeP/qYFdmyybDSf3XOWb1NIpe67mFKINVSMoycegotg8tj4cawBb\nkgu8fnYaWHpNBGLQmamo9Q==\n-----END PRIVATE KEY-----\n";

// Google Sheet details
#define SPREADSHEET_ID "1rj2vLcalnsSHkTpSdHfRWEEddB0S8MlfYcGfwMm6IoM"
#define SHEET_NAME "Home Energy Meter"

// Time server settings
const char ntpServer[] = "pool.ntp.org";
const long gmtOffset_sec = 6 * 3600; // GMT+6 for Bangladesh
const int daylightOffset_sec = 0;

// EnergyMonitor instance
EnergyMonitor emon;

// Timer for regular updates
BlynkTimer timer;

// Variables for energy calculation
float kWh = 0.0;
unsigned long lastMillis = 0;

// EEPROM addresses for each variable
const int addrVrms = 0;
const int addrIrms = 4;
const int addrPower = 8;
const int addrKWh = 12;

// Connection retry variables
int wifiRetries = 0;
const int maxWifiRetries = 20;
bool wifiConnected = false;

// Google Sheets related variables
bool gsheetsReady = false;
unsigned long lastGSheetsUpload = 0;
const unsigned long gSheetsInterval = 60000; // Upload to Google Sheets every 60 seconds

// Function prototypes
void sendEnergyDataToBlynk();
void readEnergyDataFromEEPROM();
void saveEnergyDataToEEPROM();
void connectWiFi();
void connectBlynk();
void initializeGoogleSheets();
void uploadToGoogleSheets();
String getFormattedTimestamp();
void tokenStatusCallback(TokenInfo info);

void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println("Starting IoT Energy Meter...");

#ifdef ESP32
  setCpuFrequencyMhz(80);
#endif

  EEPROM.begin(32);
  readEnergyDataFromEEPROM();

  emon.voltage(35, vCalibration, 1.7);
  emon.current(34, currCalibration);

  connectWiFi();

  if (wifiConnected) {
    Serial.println("Initializing time...");
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Failed to obtain time");
    } else {
      Serial.println("Time synchronized");
    }
  }

  if (wifiConnected) {
    connectBlynk();
    initializeGoogleSheets();
  }

  timer.setInterval(10000L, sendEnergyDataToBlynk);
  lastMillis = millis();
  Serial.println("Setup completed!");
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED) {
    wifiConnected = false;
    Serial.println("WiFi disconnected, attempting to reconnect...");
    connectWiFi();
  }

  if (wifiConnected && Blynk.connected()) {
    Blynk.run();
  }
  else if (wifiConnected && !Blynk.connected()) {
    Serial.println("Blynk not connected, retrying...");
    connectBlynk();
  }

  timer.run();

  if (wifiConnected && gsheetsReady && (millis() - lastGSheetsUpload > gSheetsInterval)) {
    uploadToGoogleSheets();
    lastGSheetsUpload = millis();
  }
  delay(10);
}

void connectWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi");
  wifiRetries = 0;
  while (WiFi.status() != WL_CONNECTED && wifiRetries < maxWifiRetries) {
    delay(500);
    Serial.print('.');
    wifiRetries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println();
    Serial.print("WiFi connected! IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    wifiConnected = false;
    Serial.println();
    Serial.println("WiFi connection failed!");
  }
}

void connectBlynk()
{
  if (wifiConnected) {
    Serial.println("Connecting to Blynk...");
    Blynk.config(auth);

    if (Blynk.connect(10000)) {
      Serial.println("Connected to Blynk!");
    } else {
      Serial.println("Failed to connect to Blynk!");
    }
  }
}

void sendEnergyDataToBlynk()
{
  emon.calcVI(10, 1000);
  unsigned long currentMillis = millis();
  if (currentMillis >= lastMillis) {
    kWh += emon.apparentPower * (currentMillis - lastMillis) / 3600000000.0;
  }
  lastMillis = currentMillis;

  Serial.printf("Vrms: %.2fV\tIrms: %.4fA\tPower: %.4fW\tkWh: %.5fkWh\n",
                emon.Vrms, emon.Irms, emon.apparentPower, kWh);

  static int saveCounter = 0;
  saveCounter++;
  if (saveCounter >= 6) {
    saveEnergyDataToEEPROM();
    saveCounter = 0;
  }

  if (wifiConnected && Blynk.connected()) {
    Blynk.virtualWrite(V0, emon.Vrms);
    Blynk.virtualWrite(V1, emon.Irms);
    Blynk.virtualWrite(V2, emon.apparentPower);
    Blynk.virtualWrite(V3, kWh);
    Serial.println("Blynk data sent");
  } else {
    Serial.println("Blynk not connected, skipping data transmission");
  }
}

void readEnergyDataFromEEPROM()
{
  EEPROM.get(addrKWh, kWh);

  if (isnan(kWh) || kWh < 0) {
    kWh = 0.0;
    Serial.println("Initialized kWh to 0.0");
    saveEnergyDataToEEPROM();
  } else {
    Serial.printf("Loaded kWh from EEPROM: %.5f\n", kWh);
  }
}

void saveEnergyDataToEEPROM()
{
  EEPROM.put(addrKWh, kWh);
  if (EEPROM.commit()) {
    Serial.println("Data saved to EEPROM");
  } else {
    Serial.println("Failed to save data to EEPROM");
  }
}

void initializeGoogleSheets()
{
  Serial.println("Initializing Google Sheets...");
  GSheet.setTokenCallback(tokenStatusCallback);
  GSheet.setPrerefreshSeconds(10 * 60);
  GSheet.begin(CLIENT_EMAIL, PROJECT_ID, PRIVATE_KEY);
  gsheetsReady = true;
  Serial.println("Google Sheets initialized");
}

void uploadToGoogleSheets()
{
  if (!gsheetsReady) return;

  Serial.println("Uploading data to Google Sheets...");

  FirebaseJson response;
  FirebaseJson valueRange;
  String timestamp = getFormattedTimestamp();

  valueRange.add("majorDimension", "ROWS");
  valueRange.set("values/[0]/[0]", timestamp);
  valueRange.set("values/[0]/[1]", emon.Vrms);
  valueRange.set("values/[0]/[2]", emon.Irms);
  valueRange.set("values/[0]/[3]", emon.apparentPower);
  valueRange.set("values/[0]/[4]", kWh);

  bool success = GSheet.values.append(&response, SPREADSHEET_ID, SHEET_NAME, &valueRange);

  if (success) {
    Serial.println("Data uploaded to Google Sheets successfully");
    Serial.printf("Uploaded: %s, %.2fV, %.4fA, %.4fW, %.5fkWh\n",
                  timestamp.c_str(), emon.Vrms, emon.Irms, emon.apparentPower, kWh);
  } else {
    Serial.println("Failed to upload data to Google Sheets");
    Serial.println(GSheet.errorReason());
  }
}

String getFormattedTimestamp()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Time_Error";
  }

  char timeString[64];
  strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timeString);
}

void tokenStatusCallback(TokenInfo info)
{
  if (info.status == token_status_error) {
    Serial.printf("Token info: type = %s, status = %s\n",
                  GSheet.getTokenType(info).c_str(),
                  GSheet.getTokenStatus(info).c_str());
    Serial.printf("Token error: %s\n", GSheet.getTokenError(info).c_str());
    gsheetsReady = false;
  } else {
    Serial.printf("Token info: type = %s, status = %s\n",
                  GSheet.getTokenType(info).c_str(),
                  GSheet.getTokenStatus(info).c_str());
    gsheetsReady = true;
  }
}

