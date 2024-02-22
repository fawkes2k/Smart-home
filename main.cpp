#include "objects.h"
#include <EEPROM.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <OneWire.h>
#include <ArduinoJson.h>

#define MAX_VALUE 4095
#define LED1_PIN 32
#define LED2_PIN 33
#define PHOTO_PIN 34
#define TEMP_PIN 5

OneWire oneWire(TEMP_PIN);
DallasTemperature sensors(&oneWire);
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
std::vector<std::unique_ptr<AElement>> elements;
uint clients;

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    JsonDocument json;
    deserializeJson(json, (char*)data);
    for (const auto& element : elements) {
      String name = element->get_name();
      if (json.containsKey(name)) element->set_value(json[name]);
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      clients += 1;
      Serial.printf("WebSocket client #%u connected from %s\n", clients, client->remoteIP().toString().c_str());
      EEPROM.writeUInt(96, clients);
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", clients);
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void setup() {
  Serial.begin(9200);
  EEPROM.begin(100);
  String SSID = EEPROM.readString(0), PASSWORD = EEPROM.readString(32);
  clients = EEPROM.readUInt(96);
  elements.push_back(std::unique_ptr<AElement>(new HTML()));
  elements.push_back(std::unique_ptr<AElement>(new Diode(LED1_PIN)));
  elements.push_back(std::unique_ptr<AElement>(new Diode(LED2_PIN)));
  elements.push_back(std::unique_ptr<AElement>(new DS18B20(&sensors, 0)));
  elements.push_back(std::unique_ptr<AElement>(new Photoresistor(PHOTO_PIN, MAX_VALUE)));
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());

  ws.onEvent(onEvent);  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = "";
    for (const auto& element : elements) html += element->print_HTML();
    request->send(200, "text/html", html);
  });
  server.addHandler(&ws);
  server.begin();
}

void loop() {
  ws.cleanupClients();
  String json;
  JsonDocument jd;
  for (const auto& element : elements) {
    String name = element->get_name(); 
    if (name == "HTML") continue;
    jd[name] = element->get_value();
  }
  serializeJson(jd, json);
  ws.textAll(json);
  delay(100);
}