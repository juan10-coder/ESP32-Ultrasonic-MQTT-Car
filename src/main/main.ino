#include <WiFi.h>
#include <WebServer.h>
#include <PubSubClient.h>
#include "config.h"  // ← pines, WiFi y MQTT definidos por preprocesador

WiFiClient espClient;
PubSubClient client(espClient);
WebServer server(80);

const int CH_IN1 = 0;
const int CH_IN2 = 1;
const int CH_IN3 = 2;
const int CH_IN4 = 3;

const int LEDC_FREQ = 1000;
const int LEDC_RES  = 8;

void stopCar() {
  ledcWrite(CH_IN1, 0);
  ledcWrite(CH_IN2, 0);
  ledcWrite(CH_IN3, 0);
  ledcWrite(CH_IN4, 0);
  digitalWrite(LED_FRONT_LEFT,  LOW);
  digitalWrite(LED_FRONT_RIGHT, LOW);
  digitalWrite(LED_BACK_LEFT,   LOW);
  digitalWrite(LED_BACK_RIGHT,  LOW);
}

void moveForward(int speed) {
  ledcWrite(CH_IN1, speed);
  ledcWrite(CH_IN2, 0);
  ledcWrite(CH_IN3, speed);
  ledcWrite(CH_IN4, 0);
  digitalWrite(LED_FRONT_LEFT,  HIGH);
  digitalWrite(LED_FRONT_RIGHT, HIGH);
  digitalWrite(LED_BACK_LEFT,   LOW);
  digitalWrite(LED_BACK_RIGHT,  LOW);
}

void moveBackward(int speed) {
  ledcWrite(CH_IN1, 0);
  ledcWrite(CH_IN2, speed);
  ledcWrite(CH_IN3, 0);
  ledcWrite(CH_IN4, speed);
  digitalWrite(LED_BACK_LEFT,  HIGH);
  digitalWrite(LED_BACK_RIGHT, HIGH);
  digitalWrite(LED_FRONT_LEFT, LOW);
  digitalWrite(LED_FRONT_RIGHT,LOW);
}

void turnLeft(int speed) {
  ledcWrite(CH_IN1, 128);
  ledcWrite(CH_IN2, 0);
  ledcWrite(CH_IN3, speed);
  ledcWrite(CH_IN4, 0);
  digitalWrite(LED_FRONT_LEFT, HIGH);
  digitalWrite(LED_FRONT_RIGHT,LOW);
}

void turnRight(int speed) {
  ledcWrite(CH_IN1, speed);
  ledcWrite(CH_IN2, 0);
  ledcWrite(CH_IN3, 128);
  ledcWrite(CH_IN4, 0);
  digitalWrite(LED_FRONT_RIGHT, HIGH);
  digitalWrite(LED_FRONT_LEFT,  LOW);
}

void handleMove() {
  if (!server.hasArg("direccion") || !server.hasArg("velocidad") || !server.hasArg("duracion")) {
    server.send(400, "application/json", "{\"error\":\"Faltan parámetros\"}");
    return;
  }

  String direccion = server.arg("direccion");
  int velocidad = constrain(server.arg("velocidad").toInt(), 0, 255);
  int duracion = server.arg("duracion").toInt();
  if (duracion > 5) duracion = 5;

  if      (direccion == "adelante")  moveForward(velocidad);
  else if (direccion == "atras")     moveBackward(velocidad);
  else if (direccion == "izquierda") turnLeft(velocidad);
  else if (direccion == "derecha")   turnRight(velocidad);
  else {
    server.send(400, "application/json", "{\"error\":\"Dirección inválida\"}");
    return;
  }

  String mensaje = "{\"direccion\":\"" + direccion +
                   "\",\"velocidad\":" + String(velocidad) + ",\"duracion\":" + String(duracion) + "}";
  client.publish(MQTT_TOPIC_MOV, mensaje.c_str());
  server.send(200, "application/json", "{\"status\":\"Movimiento ejecutado\"}");
  delay(duracion * 1000);
  stopCar();
}

void handleStatus() {
  server.send(200, "application/json", "{\"status\":\"Servidor operativo\"}");
}

void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) delay(500);
}

void reconnectMQTT() {
  while (!client.connected()) client.connect(MQTT_CLIENT_ID);
}

float mockUltrasonic() {
  float distancia = random(10, 200) + (random(0, 100) / 100.0f);
  return distancia;
}

void publishMockUltrasonic() {
  float d = mockUltrasonic();
  String payload = "{\"distancia_cm\":" + String(d, 2) + "}";
  client.publish(MQTT_TOPIC_DIST, payload.c_str());
}

unsigned long lastUltrasonicTime = 0;
const unsigned long ultrasonicInterval = 5000;

void setup() {
  Serial.begin(115200);
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  pinMode(IN3_PIN, OUTPUT);
  pinMode(IN4_PIN, OUTPUT);
  pinMode(LED_FRONT_LEFT, OUTPUT);
  pinMode(LED_FRONT_RIGHT, OUTPUT);

  ledcSetup(CH_IN1, LEDC_FREQ, LEDC_RES);
  ledcSetup(CH_IN2, LEDC_FREQ, LEDC_RES);
  ledcSetup(CH_IN3, LEDC_FREQ, LEDC_RES);
  ledcSetup(CH_IN4, LEDC_FREQ, LEDC_RES);
  ledcAttachPin(IN1_PIN, CH_IN1);
  ledcAttachPin(IN2_PIN, CH_IN2);
  ledcAttachPin(IN3_PIN, CH_IN3);
  ledcAttachPin(IN4_PIN, CH_IN4);

  stopCar();
  connectWiFi();
  client.setServer(MQTT_SERVER, MQTT_PORT);
  server.on("/move",   handleMove);
  server.on("/status", handleStatus);
  server.begin();
}

void loop() {
  if (!client.connected()) reconnectMQTT();
  client.loop();
  server.handleClient();
  unsigned long now = millis();
  if (now - lastUltrasonicTime >= ultrasonicInterval) {
    lastUltrasonicTime = now;
    publishMockUltrasonic();
  }
}