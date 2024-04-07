#include <Wire.h>
#include <vector>
#include <WiFi.h>
#include <HTTPClient.h>

#define WifiLED 8
#define RecordingLED 9
#define WIFI_SSID "Nadula"
#define WIFI_PWD "nadula123"
#define apiEndpoint "http://192.168.43.92:5000/process_data"
bool WifiDisconnectFlag = false;

float RateRoll, RatePitch, RateYaw;
float RateCalibrationRoll, RateCalibrationPitch, RateCalibrationYaw, RateCalibrationAccX, RateCalibrationAccY, RateCalibrationAccZ;
int RateCalibrationNumber;
float AccX, AccY, AccZ;
float AngleRoll, AnglePitch;
float LoopTimer;
float RateRollArray[380];
float RatePitchArray[380];
float RateYawArray[380];
float AccXArray[380];
float AccYArray[380];
float AccZArray[380];

int recNo = 0;

int ArrayIndex = 0;
bool recording = false;
unsigned long recordStartTime;

String RateRollArrayStr = "";
String RatePitchArrayStr = "";
String RateYawArrayStr = "";
String AccXArrayStr = "";
String AccYArrayStr = "";
String AccZArrayStr = "";

HTTPClient http;

//////////////////// Wifi Interrupts ////////////////////
void WiFiReady(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("WiFi Ready");
  digitalWrite(WifiLED, HIGH);
}
void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("Connected to AP successfully!");
  digitalWrite(WifiLED,LOW);
}
void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info){
  WifiDisconnectFlag = false;
  
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
}
void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("Disconnected from WiFi access point");
  Serial.print("WiFi lost connection. Reason: ");
  Serial.println(info.wifi_sta_disconnected.reason);
  WifiDisconnectFlag = true;
}

////////////////////// Function - Read MPU6050 Data //////////////////////
void gyro_signals(void) {
  Wire.beginTransmission(0x68);
  Wire.write(0x1A);
  Wire.write(0x06);
  Wire.endTransmission();
  Wire.beginTransmission(0x68);
  Wire.write(0x1C);
  Wire.write(0x10);
  Wire.endTransmission();
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.endTransmission(); 
  Wire.requestFrom(0x68,6);
  int16_t AccXLSB = Wire.read() << 8 | Wire.read();
  int16_t AccYLSB = Wire.read() << 8 | Wire.read();
  int16_t AccZLSB = Wire.read() << 8 | Wire.read();
  Wire.beginTransmission(0x68);
  Wire.write(0x1B); 
  Wire.write(0x8);                                                   
  Wire.endTransmission();
  Wire.beginTransmission(0x68);
  Wire.write(0x43);
  Wire.endTransmission();
  Wire.requestFrom(0x68,6);
  int16_t GyroX=Wire.read()<<8 | Wire.read();
  int16_t GyroY=Wire.read()<<8 | Wire.read();
  int16_t GyroZ=Wire.read()<<8 | Wire.read();
  RateRoll=(float)GyroX/65.5;
  RatePitch=(float)GyroY/65.5;
  RateYaw=(float)GyroZ/65.5;
  AccX=(float)AccXLSB/4096;
  AccY=(float)AccYLSB/4096;
  AccZ=(float)AccZLSB/4096;
  AngleRoll=atan(AccY/sqrt(AccX*AccX+AccZ*AccZ))*1/(3.142/180);
  AnglePitch=-atan(AccX/sqrt(AccY*AccY+AccZ*AccZ))*1/(3.142/180);
}

////////////////////// Function - Process Data To Send //////////////////////

void arrayToStringWithDecimal(float* array, int arraySize, String& resultStr) {
  for (int i = 0; i < arraySize; i++) {
    resultStr += String(array[i], 2); // Use 2 decimal places
    if (i < arraySize - 1) {
      resultStr += ","; // Add a comma and space to separate values
    }
  }
}

void processData(){
  arrayToStringWithDecimal(RateRollArray, sizeof(RateRollArray) / sizeof(RateRollArray[0]), RateRollArrayStr);
  arrayToStringWithDecimal(RatePitchArray, sizeof(RatePitchArray) / sizeof(RatePitchArray[0]), RatePitchArrayStr);
  arrayToStringWithDecimal(RateYawArray, sizeof(RateYawArray) / sizeof(RateYawArray[0]), RateYawArrayStr);
  arrayToStringWithDecimal(AccXArray, sizeof(AccXArray) / sizeof(AccXArray[0]), AccXArrayStr);
  arrayToStringWithDecimal(AccYArray, sizeof(AccYArray) / sizeof(AccYArray[0]), AccYArrayStr);
  arrayToStringWithDecimal(AccZArray, sizeof(AccZArray) / sizeof(AccZArray[0]), AccZArrayStr);
}
/*
void processData(){
  for (int i = 0; i < (sizeof(RateRollArray) / sizeof(RateRollArray[0])); i++) {
    RateRollArrayStr += String(RateRollArray[i], 2); // Use 2 decimal places
    if (i < sizeof(RateRollArray) / sizeof(RateRollArray[0]) - 1) {
      RateRollArrayStr += ","; // Add a comma and space to separate values
    }
  }
  for (int i = 0; i < sizeof(RatePitchArray) / sizeof(RatePitchArray[0]); i++) {
    RatePitchArrayStr += String(RatePitchArray[i], 2); // Use 2 decimal places
    if (i < sizeof(RatePitchArray) / sizeof(RatePitchArray[0]) - 1) {
      RatePitchArrayStr += ","; // Add a comma and space to separate values
    }
  }
  for (int i = 0; i < sizeof(RateYawArray) / sizeof(RateYawArray[0]); i++) {
    RateYawArrayStr += String(RateYawArray[i], 2); // Use 2 decimal places
    if (i < sizeof(RateYawArray) / sizeof(RateYawArray[0]) - 1) {
      RateYawArrayStr += ","; // Add a comma and space to separate values
    }
  }
  for (int i = 0; i < sizeof(AccXArray) / sizeof(AccXArray[0]); i++) {
    AccXArrayStr += String(AccXArray[i], 2); // Use 2 decimal places
    if (i < sizeof(AccXArray) / sizeof(AccXArray[0]) - 1) {
      AccXArrayStr += ","; // Add a comma and space to separate values
    }
  }
  for (int i = 0; i < sizeof(AccYArray) / sizeof(AccYArray[0]); i++) {
    AccYArrayStr += String(AccYArray[i], 2); // Use 2 decimal places
    if (i < sizeof(AccYArray) / sizeof(AccYArray[0]) - 1) {
      AccYArrayStr += ","; // Add a comma and space to separate values
    }
  }
  for (int i = 0; i < sizeof(AccZArray) / sizeof(AccZArray[0]); i++) {
    AccZArrayStr += String(AccZArray[i], 2); // Use 2 decimal places
    if (i < sizeof(AccZArray) / sizeof(AccZArray[0]) - 1) {
      AccZArrayStr += ","; // Add a comma and space to separate values
    }
  }
}*/

////////////////////// Function - Send Data To API //////////////////////
void sendData(){
  if (WiFi.status() == WL_CONNECTED) {
    // Create a JSON payload
    String jsonData = "{\"RateRollArray\":\"" + RateRollArrayStr + 
                        "\",\"RatePitchArray\":\"" + RatePitchArrayStr + 
                        "\",\"RateYawArray\":\"" + RateYawArrayStr + 
                        "\",\"AccXArray\":\"" + AccXArrayStr + 
                        "\",\"AccYArray\":\"" + AccYArrayStr + 
                        "\",\"AccZArray\":\"" + AccZArrayStr + 
                        "\",\"recNo\":\"" + recNo +"\"}";
    http.begin(apiEndpoint);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(jsonData);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
    } else {
      Serial.println("Error on HTTP request");
      sendData();
      delay(1000);
    }
    
    http.end();
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(4,5); //sda-4, scl-5
  delay(250);
  Wire.beginTransmission(0x68); 
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();

  Serial.println("1");
  // IndicatorLED Pins Configuration
  pinMode(WifiLED, OUTPUT);
  digitalWrite(WifiLED, LOW);
  pinMode(RecordingLED, OUTPUT);
  digitalWrite(RecordingLED, LOW);
  Serial.println("2");
  // MPU6050 Calibration
  for (RateCalibrationNumber=0;RateCalibrationNumber<2000;RateCalibrationNumber ++) {
    gyro_signals();
    RateCalibrationRoll+=RateRoll;
    RateCalibrationPitch+=RatePitch;
    RateCalibrationYaw+=RateYaw;
    RateCalibrationAccX+=AccX;
    RateCalibrationAccY+=AccY;
    RateCalibrationAccZ+=AccZ;
    delay(1);
  }
  RateCalibrationRoll/=2000;
  RateCalibrationPitch/=2000;
  RateCalibrationYaw/=2000;   
  RateCalibrationAccX/=2000;
  RateCalibrationAccY/=2000;
  RateCalibrationAccZ/=2000;
  Serial.println("3");
  // WIFI Events
  WiFi.onEvent(WiFiReady, WiFiEvent_t::ARDUINO_EVENT_WIFI_READY);
  WiFi.onEvent(WiFiStationConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
  WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  WiFi.disconnect(true);
  delay(1000);
  Serial.println("4");
  WiFi.mode(WIFI_STA); // Mode Selection
  Serial.println("5");
  WiFi.begin(WIFI_SSID, WIFI_PWD); // Connect to AP
  Serial.println("6");
}

void loop() {
  // Reconnecting to the AP if disconnected
  while(WifiDisconnectFlag){
    Serial.println("Trying to Reconnect");
    WiFi.begin(WIFI_SSID, WIFI_PWD);
    digitalWrite(WifiLED, !digitalRead(WifiLED)); // Flash Wifi Indicator
    delay(1000);
  }

  if(Serial.available()){
    String input = Serial.readString();
    input.trim();
    if(input == "start"){
      Serial.println("Recording Started !");
      recording = true;
      recordStartTime = millis();
    }else if(input == "reset"){
      Serial.println("Recording Reseted !");
      recNo = 0;
    }else{
      Serial.print("Invalid Input: ");
      Serial.println(input);
    }
  }

  if(recording){
    if(millis()-recordStartTime<20000){
      gyro_signals();
      RateRoll-=RateCalibrationRoll; 
      RatePitch-=RateCalibrationPitch; 
      RateYaw-=RateCalibrationYaw; 
      AccX-=RateCalibrationAccX; 
      AccY-=RateCalibrationAccY; 
      AccZ-=RateCalibrationAccZ; 
      
      // Adding Data To Arrays
      RateRollArray[ArrayIndex] = RateRoll;
      RatePitchArray[ArrayIndex] = RatePitch;
      RateYawArray[ArrayIndex] = RateYaw;
      AccXArray[ArrayIndex] = AccX;
      AccYArray[ArrayIndex] = AccY;
      AccZArray[ArrayIndex] = AccZ;

      ArrayIndex++;
      Serial.print("CRoll:");
      Serial.print(RateRoll); 
      Serial.print(",");
      Serial.print("CPitch:");
      Serial.print(RatePitch);
      Serial.print(",");
      Serial.print("CYaw:");
      Serial.print(RateYaw);
      Serial.print(",");
      Serial.print("CAccX:");
      Serial.print(AccX); 
      Serial.print(",");
      Serial.print("CAccY:");
      Serial.print(AccY);
      Serial.print(",");
      Serial.print("CAccZ:");
      Serial.println(AccZ);
      //AngleRoll
      //AnglePitch
      digitalWrite(RecordingLED, !digitalRead(RecordingLED));

    }else{
      // Print Array Size
      Serial.print("Data Count: ");
      Serial.println(ArrayIndex+1);

      // Print Arrays
      Serial.print("RateRollArray = [");
      for(int i = 0; i < (ArrayIndex+1); i++){
        Serial.print(RateRollArray[i]);
        Serial.print(",");
      }
      Serial.println("]");
      Serial.print("RatePitchArray = [");
      for(int i = 0; i < (ArrayIndex+1); i++){
        Serial.print(RatePitchArray[i]);
        Serial.print(",");
      }
      Serial.println("]");
      Serial.print("RateYawArray = [");
      for(int i = 0; i < (ArrayIndex+1); i++){
        Serial.print(RateYawArray[i]);
        Serial.print(",");
      }
      Serial.println("]");
      Serial.print("AccXArray = [");
      for(int i = 0; i < (ArrayIndex+1); i++){
        Serial.print(AccXArray[i]);
        Serial.print(",");
      }
      Serial.println("]");
      Serial.print("AccYArray = [");
      for(int i = 0; i < (ArrayIndex+1); i++){
        Serial.print(AccYArray[i]);
        Serial.print(",");
      }
      Serial.println("]");
      Serial.print("AccZArray = [");
      for(int i = 0; i < (ArrayIndex+1); i++){
        Serial.print(AccZArray[i]);
        Serial.print(",");
      }
      Serial.println("]");

      recNo++;
      processData();
      Serial.print("RateRollArrayStr:");
      Serial.println(RateRollArrayStr);
      sendData();

      recording = false; // Reset Flag
      ArrayIndex = 0; // Reset Array Index
      digitalWrite(RecordingLED, LOW); // Flash Recording Indicator
      RateRollArrayStr = "";
      RatePitchArrayStr = "";
      RateYawArrayStr = "";
      AccXArrayStr = "";
      AccYArrayStr = "";
      AccZArrayStr = "";
    }

    if (recNo==4){
      recNo = 0;
    }
  }
  
  delay(50);
}