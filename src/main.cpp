#include <Arduino.h>
#include <DHT.h>
#include <PubSubClient.h>
#include <WiFi.h>

// ==================================
//            Setup WiFi
// ==================================
#define WIFI_NAME "Wokwi-GUEST"
#define WIFI_PASS ""

// ==================================
//            Setup MQTT
// ==================================
WiFiClient mqttClient;
PubSubClient mqtt(mqttClient);

#define MQTT_SERVER "broker.emqx.io"
#define MQTT_PORT 1883
#define MQTT_CLIENT "62051916"                   // <---- แก้ตรงนี้ ⚠️

#define SEND_TEMP_TOPIC  "62051916/temperature"   // <---- แก้ตรงนี้ ⚠️
#define SEND_HUMI_TOPIC   "62051916/humidity"    // <---- แก้ตรงนี้ ⚠️
#define  CTRL_RED_TOPIC   "62051916/red"             // <---- แก้ตรงนี้ ⚠️
#define  CTRL_GREEN_TOPIC  "62051916/green"   // <---- แก้ตรงนี้ ⚠️


// ==================================
//          Define DHT22 Pin
// ==================================
#define DHT_PIN 4
#define DHT_TYPE DHT22
DHT dht(DHT_PIN, DHT_TYPE);

// ==================================
//           Define LED Pin
// ==================================
#define LED_RED   25
#define LED_GREEN 26
#define LED_BLUE  27

void connectWiFi();
void setupServerMQTT();
void connectMQTT();
void callback(char* topic, byte* payload, unsigned int length);

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_BLUE, HIGH);

  dht.begin();

  connectWiFi();
  setupServerMQTT();

}

void loop() {

  connectMQTT();

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.println(F("°C "));

  mqtt.publish(SEND_TEMP_TOPIC, String(t).c_str());
  mqtt.publish(SEND_HUMI_TOPIC, String(h).c_str());

  mqtt.loop();
  delay(1000);
}

void connectWiFi()
{
  Serial.println("Connecting to WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_NAME, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println(" Connected!");
}

void setupServerMQTT()
{
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(callback);
}

void connectMQTT()
{
  while(!mqtt.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt.connect(MQTT_CLIENT)) {
      Serial.println("connected");

      mqtt.subscribe(CTRL_RED_TOPIC);         // <---- subscribe topic led 🔴
      mqtt.subscribe(CTRL_GREEN_TOPIC);    // <---- subscribe topic led 🟢
                                                                      // <---- subscribe topic led 🔵

    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(" | Message: ");

  String payload_str = "";
  String topic_str = String(topic);

  // แปลง payload จาก byte เป็น String
  for (int i = 0; i < length; i++) {
    char c = (char)payload[i];
    Serial.print(c);
    payload_str += c;
  }
  Serial.println();

  if(topic_str == CTRL_RED_TOPIC)     // <---- ตรวจสอบ topic ที่ได้รับ ✅
  {
    if(payload_str == "ON")           // <---- ตรวจสอบ patlod ที่ได้รับ ✅
    {
      digitalWrite(LED_RED, LOW);     // <---- เปิดหลอด LED 🔴
    }
    else
    {
      digitalWrite(LED_RED, HIGH);    // <---- ปิดหลอด LED ⚪️
    }
  }
  else if(topic_str == CTRL_GREEN_TOPIC)    // <---- ตรวจสอบ topic ที่ได้รับ ✅
  {
    if(payload_str == "ON")                 // <---- ตรวจสอบ patlod ที่ได้รับ ✅
    {
      digitalWrite(LED_GREEN, LOW);         // <---- เปิดหลอด LED 🟢
    }
    else
    {
      digitalWrite(LED_GREEN, HIGH);        // <---- ปิดหลอด LED ⚪️
    }
  }
                                            // <---- ไปทำ led สีน้ำเงินเพิ่มมาซะ 😄
}

