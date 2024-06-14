#include <WiFi.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
#include "DHT.h"
#include <DallasTemperature.h>
#include <OneWire.h>

#define DHTPIN 33
#define TYPE DHT22

DHT dht(DHTPIN, TYPE);

const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* mqtt_topic = "/sensor/data";
const char* mqtt_topic_pub = "/sensor/data";
WiFiClient espClient;
PubSubClient client(espClient);

int PIR = 25;
int sensorPin = 16;

OneWire oneWire(sensorPin); // creates the OneWire object using a specific pin
DallasTemperature sensor(&oneWire);

void callback(char* topic, byte* payload, unsigned int length) {
  if (length == 0) return;
  String msg = "";
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  Serial.print("Received Message: ");
  Serial.println(msg);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("alpskkmmdwodw")) {
      Serial.println("connected");
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIR, INPUT);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  sensor.begin();
}

String readTemperatureHumidity() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    return "No data read!";
  }

  String msg = "Humidity: ";
  msg += String(humidity);
  return msg;
}

String readPIR() {
  int PIRstatus = digitalRead(PIR);
  if (PIRstatus == 0) {
    return "tidak ada orang";
  } else {
    return "ada orang, hati-hati!";
  }
}

String readDS() {
  float temp = sensor.getTempCByIndex(0);
  if (isnan(temp)) {
    return "No data read!";
  }
  String msgDs = "Temperature: ";
  msgDs += String(temp);

  return msgDs;
}

void publishMessage(const char* message) {
  if (client.connected()) {
    client.publish(mqtt_topic_pub, message);
    Serial.println("Message sent to MQTT topic");
  } else {
    Serial.println("Failed to publish message. MQTT client not connected.");
  }
}

void sendToFlaskServer(String temperature, String humidity, String pirStatus) {
  if (WiFi.status() == WL_CONNECTED) { // Check WiFi connection status
    HTTPClient http;

    http.begin("http:192.168.100.35:5000/sensor/data"); // Your Flask server URL
    http.addHeader("Content-Type", "application/json");

    String jsonData = "{\"temperature\":\"" + temperature + "\", \"humidity\":\"" + humidity + "\", \"pir_status\":\"" + pirStatus + "\"}";

    int httpResponseCode = http.POST(jsonData);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode); // Print return code
      Serial.println(response); // Print request answer
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }

    http.end(); // Free resources
  } else {
    Serial.println("Error in WiFi connection");
  }
}

void loop() {
  String msg = readTemperatureHumidity();
  String msg2 = readPIR();
  String msg3 = readDS();
  String msg_send = msg + ", " + msg2 + ", " + msg3;
  sensor.requestTemperatures();
  publishMessage(msg_send.c_str());

  // Extract temperature, humidity, and PIR status
  String temperature = String(dht.readTemperature());
  String humidity = String(dht.readHumidity());
  String pirStatus = readPIR();

  sendToFlaskServer(temperature, humidity, pirStatus);

  delay(3000);
  if (!client.connected()) {
    reconnect();
  }

  client.loop();
  delay(2000);
}
