#include <SPI.h>   // library for SPI Communication

#include <Wire.h>   //Library for I2C Communication

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "secrets.h"

#define AWS_IOT_PUBLISH_TOPIC   "ESP8266/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "ESP8266/sub"

unsigned long lastMillis = 0;
unsigned long previousMillis = 0;
const long interval = 5000;
struct tm timeinfo;

int pin_Out_S0 = 5;
int pin_Out_S1 = 4;
int pin_Out_S2 = 14;
int pin_In_Mux1 = A0;
int Mux1_State[8]={0};

int mq_data[8]={0}; //empty string variable to hold all data untill it is published
String mystring;

//AWS Certification

WiFiClientSecure net;
 
BearSSL::X509List cert(AWS_CERT_CA);
BearSSL::X509List client_crt(AWS_CERT_CRT);
BearSSL::PrivateKey key(AWS_CERT_PRIVATE);
 
PubSubClient client(net);
 
time_t now;
time_t nowish = 1510592825;
 
//NTP Connect 
void NTPConnect(void)
{
  Serial.print("Setting time using SNTP");
  configTime(TIME_ZONE * 3600, 0 * 3600, "pool.ntp.org", "time.nist.gov");
  now = time(nullptr);
  while (now < nowish)
  {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("done!");

  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}
 
 
void messageReceived(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Received [");
  Serial.print(topic);
  Serial.print("]: ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
 
 
void connectAWS()
{
  delay(3000);
  // Connecting to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
  Serial.println(String("Attempting to connect to SSID: ") + String(WIFI_SSID));
 
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);
  }
 
  NTPConnect();
 
  net.setTrustAnchors(&cert);
  net.setClientRSACert(&client_crt, &key);
 
  client.setServer(AWS_IOT_ENDPOINT, 8883);
  client.setCallback(messageReceived);
 
 
  Serial.println("Connecting to AWS IOT");
 
  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(1000);
  }
 
  if (!client.connected()) {
    Serial.println("AWS IoT Timeout!");
    return;
  }
  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
 
  Serial.println("AWS IoT Connected!");
}
 
 
void publishMessage()
{
  StaticJsonDocument<200> doc;

  gmtime_r(&now, &timeinfo);
  doc["Time"] = asctime(&timeinfo);
  doc["MQ2"] = mq_data[0];
  doc["MQ6"] = mq_data[1];
  doc["MQ8"] = mq_data[2];
  doc["MQ3"] = mq_data[3];
  doc["MQ4"] = mq_data[4];
  doc["MQ9"] = mq_data[5];
  doc["MQ135"] = mq_data[6];
  doc["MQ7"] = mq_data[7];
  char jsonBuffer[1024];
  serializeJson(doc, jsonBuffer); // print to client
 
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}

//AWS CODE ENDS HERE
void setup() {
  pinMode(pin_Out_S0, OUTPUT);
  pinMode(pin_Out_S1, OUTPUT);
  pinMode(pin_Out_S2, OUTPUT);
  pinMode(pin_In_Mux1, INPUT);
  Serial.begin(9600);  
  Wire.begin(); 
  connectAWS();

  Serial.println();
}
void loop() {
  updateMux1();          
  mystring="";
  int snsr1 = Mux1_State[0];    //Fetching analog data from Sensor 1
  int snsr2 = Mux1_State[1];    //Fetching analog data from Sensor 2
  int snsr3 = Mux1_State[2];    //Fetching analog data from Sensor 3
  int snsr4 = Mux1_State[3];    //Fetching analog data from Sensor 4
  int snsr5 = Mux1_State[4];    //Fetching analog data from Sensor 5
  int snsr6 = Mux1_State[5];    //Fetching analog data from Sensor 6
  int snsr8 = Mux1_State[6];    
  int snsr7 = Mux1_State[7];
  ///////////////////////////
  mq_data[0] = Mux1_State[0];    //Fetching analog data from Sensor 1 MQ2
  mq_data[1] = Mux1_State[1];    //Fetching analog data from Sensor 2 MQ6
  mq_data[2] = Mux1_State[2];    //Fetching analog data from Sensor 3 MQ8
  mq_data[3] = Mux1_State[3];    //Fetching analog data from Sensor 4 MQ3
  mq_data[4] = Mux1_State[4];    //Fetching analog data from Sensor 5 MQ4
  mq_data[5] = Mux1_State[5];    //Fetching analog data from Sensor 6 MQ9
  mq_data[6] = Mux1_State[6];    //MQ135
  mq_data[7] = Mux1_State[7];    //MQ7
  
  // mystring += (String(snsr1) + ","+ String(snsr2) + "," + String(snsr3) + "," + String(snsr4) + "," + String(snsr5) + "," + String(snsr6)+ "," + String(snsr7)+ "," + String(snsr8));
  mystring += ("MQ2: "+String(snsr1) + ","+ "MQ6: "+String(snsr2) + "," + "MQ8: "+String(snsr3) + "," + "MQ3: "+String(snsr4) + "," + "MQ4: "+String(snsr5) + "," + "MQ9: "+String(snsr6)+ "," + "MQ135: "+String(snsr7)+ "," + "MQ7: "+String(snsr8));

  gmtime_r(&now, &timeinfo); 
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo)+'\n');
  Serial.print(mystring+'\n');
  delay(2000);
 
  now = time(nullptr);
 
  if (!client.connected())
  {
    connectAWS();
  }
  else
  {
    client.loop();
    if (millis() - lastMillis > 5000)
    {
      lastMillis = millis();
      publishMessage();
    }
  } 
}

void updateMux1 () {
  for (int i = 0; i < 8; i++){
    digitalWrite(pin_Out_S0, HIGH && (i & B00000001));
    digitalWrite(pin_Out_S1, HIGH && (i & B00000010));
    digitalWrite(pin_Out_S2, HIGH && (i & B00000100));
    Mux1_State[i] = analogRead(pin_In_Mux1);
  }
}
