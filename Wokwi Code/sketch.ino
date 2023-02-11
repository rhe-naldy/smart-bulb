
// From https://randomnerdtutorials.com/esp32-mqtt-publish-subscribe-arduino-ide
// topic to publish : "binusGroup5/led"
// topic to publish : "binusGroup5/schedButton"
// topic to publish : "binusGroup5/start"
// topic to publish : "binusGroup5/end"
// host: test.mostquitto.org:1883

#include <WiFi.h>
#include <NTPClient.h> //
#include <WiFiUdp.h> //
#include "PubSubClient.h"

const char* ssid = "Wokwi-GUEST"; // Wi-Fi SSID
const char* password = ""; // password Wi-Fi
const char* mqttServer = "test.mosquitto.org";
int port = 1883;
char clientId[50];

WiFiClient espClient;
PubSubClient client(espClient);

WiFiUDP ntpUDP; //
NTPClient timeClient(ntpUDP); //
String formattedDate; //
String timeStamp; //

String startTime; //
String endTime; //
String setScheduling; //
int scheduleStatus = 0; //

const int ledPin = 2;

void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(0));
  Serial.print(timeStamp);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  wifiConnect();

  Serial.println("");
  Serial.println("WiFi connected");

  timeClient.begin(); //
  timeClient.setTimeOffset(25200); //GMT +7
  
  client.setServer(mqttServer, port);
  client.setCallback(callback);
  pinMode(ledPin, OUTPUT);
}

void wifiConnect() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
}

void mqttReconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    long r = random(1000);
    sprintf(clientId, "clientId-%ld", r);
    if (client.connect(clientId)) {
      Serial.print(clientId);
      Serial.println(" connected");
      client.subscribe("binusGroup5/led");
      client.subscribe("binusGroup5/schedButton"); //
      client.subscribe("binusGroup5/start"); //
      client.subscribe("binusGroup5/end"); //
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String stMessage;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    stMessage += (char)message[i];
  }
  Serial.println();

  if (String(topic) == "binusGroup5/led") {
    Serial.print("Changing output to ");
    if(stMessage == "1"){
      Serial.println("on");
      digitalWrite(ledPin, HIGH);
    }
    else if(stMessage == "0"){
      Serial.println("off");
      digitalWrite(ledPin, LOW);
    }
  }

  if (String(topic) == "binusGroup5/schedButton") {
    if(stMessage == "1"){
      setScheduling = "1";
    }
    else if(stMessage == "0"){
      setScheduling = "0";
    }
  }

  if (String(topic) == "binusGroup5/start"){
    startTime = stMessage;
  }

  if (String(topic) == "binusGroup5/end"){
    endTime = stMessage;
  }
}

void scheduling(){
  if(startTime == timeStamp && scheduleStatus == 0){
    Serial.println("Start");
    digitalWrite(ledPin, HIGH);
    scheduleStatus = 1;
  }

  if(endTime == timeStamp && scheduleStatus == 1){
    Serial.println("End");
    digitalWrite(ledPin, LOW);
    scheduleStatus = 0;
  }
}

void loop() {
  timeClient.forceUpdate();
  formattedDate = timeClient.getFormattedTime();
  Serial.println(formattedDate);
  timeStamp = formattedDate.substring(0, 5);
  // Serial.print("HOUR: ");
  // Serial.println(timeStamp);

  if (setScheduling == "1"){
    scheduling();
    delay(10);
  }
  
  delay(10);
  if (!client.connected()) {
    mqttReconnect();
  }
  client.loop();
}
