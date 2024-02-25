// D0 - 16
// D1 - 5
// D2 - 4
// D3 - 0
// D4 - 2
// D5 - 14
// D6 - 12
// D7 - 13
// D8 - 15
// RX - 3
// TX - 1

#include <Wire.h>
#include <BH1750.h>
#include <math.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"

#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#define API_KEY ""
#define DB_URL ""
#define DATA_PIN 4
#define LED_PIN 14

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOk = false;
float sensorData = 0.0;
int dutyCycle = 0;
bool manual = false;

BH1750 lightMeter;

void setup(){
  Serial.begin(9600);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi...");
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.print(WiFi.localIP());
  Serial.println();

  Wire.begin();
  lightMeter.begin();
  pinMode(LED_PIN, OUTPUT);
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  Serial.println("BH1750 begin.");

  config.api_key = API_KEY;
  config.database_url = DB_URL;
  if(Firebase.signUp(&config, &auth,"","")){
    Serial.println("signUp OK");
    signupOk = true;
  }
  else
  {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  
  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config,&auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  if(Firebase.ready() && signupOk && (millis() - sendDataPrevMillis > 10 || sendDataPrevMillis == 0))
  {
    sensorData = lightMeter.readLightLevel();
    sendDataPrevMillis = millis();

    if(Firebase.RTDB.getBool(&fbdo, "manual"))
    {
      if(fbdo.dataType()=="boolean")
      {
        manual = fbdo.boolData();
        Serial.println("Successful read from "+fbdo.dataPath()+": "+manual+" ("+fbdo.dataType()+")");
      }
    }
    else
    {
      Serial.println("FAILED: " + fbdo.errorReason());
    }

    if(!manual)
    {
      if(sensorData>40)
      {
        dutyCycle = 0;
      }
      else
      {
        dutyCycle = round(254-(sensorData/0.16));
      }
    }
    else
    {
      if(Firebase.RTDB.getInt(&fbdo, "Sensor/duty_cycle"))
      {
        if(fbdo.dataType()=="int")
        {
          dutyCycle = fbdo.intData();
          Serial.println("Successful read from "+fbdo.dataPath()+": "+dutyCycle+" ("+fbdo.dataType()+")");
        }
      }
      else
      {
        Serial.println("FAILED: " + fbdo.errorReason());
      }
    }

    analogWrite(LED_PIN, dutyCycle);

    if(Firebase.RTDB.setFloat(&fbdo, "Sensor/sensor_data", sensorData))
    {
      Serial.println(); Serial.print(sensorData);
      Serial.print(" - successfully saved to: " + fbdo.dataPath());
      Serial.println(" (" + fbdo.dataType() + ")");
    }
    else
    {
      Serial.println("FAILED: " + fbdo.errorReason());
    }

    if(!manual)
    {
        if(Firebase.RTDB.setInt(&fbdo, "Sensor/duty_cycle", dutyCycle))
        {
          Serial.println(); Serial.print(dutyCycle);
          Serial.print(" - successfully saved to: " + fbdo.dataPath());
          Serial.println(" (" + fbdo.dataType() + ")");
        }
        else
        {
          Serial.println("FAILED: " + fbdo.errorReason());
        }
    }
  }
}