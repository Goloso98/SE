#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <SimpleDHT.h>
#include <PubSubClient.h>

#include "secrets.h"

#define LED D6
#define LDR A0
#define PIR D5
#define DHT D4
#define BTNTGL D3

#ifdef ENTRANCE
#define HOUSEIN D7
#define HOUSEOUT D2
#endif

#ifndef ROOMID
#define ROOMID 42
#endif

#define MSG_BUFFER_SIZE	(50)
#define PUBLISH_DELAY_MS (3000)

void publish_ldr();
void publish_pir();
void publish_dht();
void handle_switch();
#ifdef ENTRANCE
void publish_people();
#endif

const char* ssid        = SSID;
const char* password    = PASSWORD;
const char* mqtt_server = MQTT_SERVER;
const char* mqtt_user   = MQTT_USER;
const char* mqtt_pass   = MQTT_PASS;

WiFiClient sta;
PubSubClient client(sta);
#ifdef DHT11
SimpleDHT11 dht(DHT);
#endif
#ifdef DHT22
SimpleDHT22 dht(DHT);
#endif
unsigned long lastMsg = 0;
char msg[MSG_BUFFER_SIZE];
bool btntgl_last = digitalRead(BTNTGL);
bool pir_val_last = digitalRead(PIR);
#ifdef ENTRANCE
bool housein_last = digitalRead(HOUSEIN);
bool houseout_last = digitalRead(HOUSEOUT);
#endif

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == 't') {
    digitalWrite(LED, HIGH);
  } else {
    digitalWrite(LED, LOW);
  }
}

void reconnect() {
  char topic[MSG_BUFFER_SIZE];
  snprintf(topic, MSG_BUFFER_SIZE, "rooms/%d/lamp_status", ROOMID);
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      /////client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe(topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(LDR, INPUT);
  pinMode(PIR, INPUT);
  pinMode(BTNTGL, INPUT);
  #ifdef ENTRANCE
  pinMode(HOUSEIN, INPUT);
  pinMode(HOUSEOUT, INPUT);
  #endif
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > PUBLISH_DELAY_MS) {
    lastMsg = now;
    publish_ldr();
    publish_dht();
  }
  publish_pir();
  handle_switch();
  #ifdef ENTRANCE
  publish_people();
  #endif
}

void publish_ldr() {
  int ldr_val = 1024 - analogRead(LDR);
  char ldr_pub[MSG_BUFFER_SIZE];
  char topic[MSG_BUFFER_SIZE];

  snprintf(ldr_pub, MSG_BUFFER_SIZE, "%d", ldr_val);
  snprintf(topic, MSG_BUFFER_SIZE, "rooms/%d/light_sensor", ROOMID);

  client.publish(topic, ldr_pub);
  Serial.print("ldr: ");
  Serial.println(ldr_val);
}

void publish_pir() {
  char topic[MSG_BUFFER_SIZE];
  snprintf(topic, MSG_BUFFER_SIZE, "rooms/%d/presence_sensor", ROOMID);

  int pir_val = digitalRead(PIR);

  if (pir_val != pir_val_last) {
    pir_val_last = pir_val;

    if (pir_val) {
      client.publish(topic, "true");
    } else {
      client.publish(topic, "false");
    }
    
    Serial.print("pir: ");
    Serial.println(pir_val);
  }
}

void publish_dht() {
  #ifdef DHT11
  byte temperature_val = 0;
  byte humidity_val = 0;
  #endif
  #ifdef DHT22
  float temperature_val = 0;
  float humidity_val = 0;
  #endif
  char topicT[MSG_BUFFER_SIZE];
  char topicH[MSG_BUFFER_SIZE];
  snprintf(topicT, MSG_BUFFER_SIZE, "rooms/%d/temperature", ROOMID);
  snprintf(topicH, MSG_BUFFER_SIZE, "rooms/%d/humidity", ROOMID);

  char temperature_pub[MSG_BUFFER_SIZE];
  char humidity_pub[MSG_BUFFER_SIZE];
  
  int err = SimpleDHTErrSuccess;

  #ifdef DHT11
  if ((err = dht.read(DHT, &temperature_val, &humidity_val, NULL)) != SimpleDHTErrSuccess) {
    Serial.print("Read DHT11 failed.");
    return;
  }
  #endif
  #ifdef DHT22
  if ((err = dht.read2(DHT, &temperature_val, &humidity_val, NULL)) != SimpleDHTErrSuccess) {
    Serial.print("Read DHT22 failed.");
    return;
  }
  #endif

  snprintf(temperature_pub, MSG_BUFFER_SIZE, "%d", (int)temperature_val);
  snprintf(humidity_pub, MSG_BUFFER_SIZE, "%d", (int)humidity_val);

  client.publish(topicT, temperature_pub);
  client.publish(topicH, humidity_pub);

  Serial.print("temp: ");
  Serial.println(temperature_val);
  Serial.print("hum: ");
  Serial.println(humidity_val);
}

void handle_switch() {
  bool btntgl = !digitalRead(BTNTGL);
  char topic[MSG_BUFFER_SIZE];
  snprintf(topic, MSG_BUFFER_SIZE, "rooms/%d/switch_toggle", ROOMID);

  if (btntgl != btntgl_last) {
    btntgl_last = btntgl;
    if (btntgl) {
      Serial.println("Switch toggle");
      client.publish(topic, "true");
    }
    // Serial.println(topic);
  }
}

#ifdef ENTRANCE
void publish_people() {
  bool housein = !digitalRead(HOUSEIN);
  bool houseout = !digitalRead(HOUSEOUT);

  if (housein != housein_last) {
    housein_last = housein;
    if (housein) {
      Serial.println("Add person");
      client.publish("house/people/add", "true");
    }
  }

  if (houseout != houseout_last) {
    houseout_last = houseout;
    if (houseout) {
      Serial.println("Remove person");
      client.publish("house/people/remove", "true");
    }
  }
}
#endif
