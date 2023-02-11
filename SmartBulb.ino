// topic to publish : "binusGroup5/led" -> on/off toggle button
// topic to publish : "binusGroup5/schedButton" -> activate schedule feature button
// topic to publish : "binusGroup5/start" -> insert start time
// topic to publish : "binusGroup5/end" -> insert end time
// topic to subscribe: "binusGroup5/led_status" -> on/off toggle button
// topic to subscribe: "binusGroup5/sched_status" -> activate schedule feature button
// topic to subscribe: "binusGroup5/start_time" -> display set start time
// topic to subscribe: "binusGroup5/end_time" -> display set end time
// host: test.mostquitto.org:1883

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "PubSubClient.h"

const char* ssid = "ssid"; // Wi-Fi SSID
const char* password = "password"; // password Wi-Fi
const char* mqttServer = "test.mosquitto.org";
int port = 1883;
char clientId[50];

WiFiClient espClient;
PubSubClient client(espClient);

WiFiUDP ntpUDP; 
NTPClient timeClient(ntpUDP); 
String formattedDate; 
String timeStamp; 

String startTime; 
String endTime; 
String setScheduling; 
String scheduleStatus = "0"; 

const int ledPin = D2;

void setup() {
  Serial.begin(115200);
  Serial.print(timeStamp);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  wifiConnect();

  Serial.println("");
  Serial.println("WiFi connected");

  timeClient.begin(); 
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
      client.subscribe("binusGroup5/schedButton"); 
      client.subscribe("binusGroup5/start"); 
      client.subscribe("binusGroup5/end"); 
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
      digitalWrite(ledPin, LOW);
      client.publish("binusGroup5/led_status", stMessage.c_str());
    }
    else if(stMessage == "0"){
      Serial.println("off");
      digitalWrite(ledPin, HIGH);
      client.publish("binusGroup5/led_status", stMessage.c_str());
    }
  }

  if (String(topic) == "binusGroup5/schedButton") {
    if(stMessage == "1"){
      setScheduling = "1";
      client.publish("binusGroup5/sched_status", stMessage.c_str());
    }
    else if(stMessage == "0"){
      setScheduling = "0";
      client.publish("binusGroup5/sched_status", stMessage.c_str());
    }
  }

  if (String(topic) == "binusGroup5/start"){
    startTime = stMessage;
    client.publish("binusGroup5/start_time", startTime.c_str());
  }

  if (String(topic) == "binusGroup5/end"){
    endTime = stMessage;
    client.publish("binusGroup5/end_time", endTime.c_str());
  }
}

void scheduling(){
  if(startTime == timeStamp && scheduleStatus == "0"){
    Serial.println("Start");
    digitalWrite(ledPin, LOW);
    scheduleStatus = "1";
    client.publish("binusGroup5/led_status", scheduleStatus.c_str());
    client.publish("binusGroup5/sched_status", scheduleStatus.c_str());
  }

  if(endTime == timeStamp && scheduleStatus == "1"){
    Serial.println("End");
    digitalWrite(ledPin, HIGH);
    scheduleStatus = "0";
    client.publish("binusGroup5/led_status", scheduleStatus.c_str());
    client.publish("binusGroup5/sched_status", scheduleStatus.c_str());
  }
}

void loop() {
  timeClient.update();
  formattedDate = timeClient.getFormattedTime();
  Serial.println(formattedDate);
  timeStamp = formattedDate.substring(0, 5);

  if (setScheduling == "1"){
    scheduling();
    delay(100);
  }
  
  delay(100);
  if (!client.connected()) {
    mqttReconnect();
    if(digitalRead(ledPin) == HIGH){
      String msg = "0";
      client.publish("binusGroup5/led_status", msg.c_str());
    } else if (digitalRead(ledPin) == LOW){
      String msg = "1";
      client.publish("binusGroup5/led_status", msg.c_str());
    }
    
  }
  client.loop();
}
