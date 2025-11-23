#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>
#include "config.h"
#include "web_ui.h"

// ========================= CONFIG WIFI =========================
const char* WIFI_SSID  = "BAZZANIR";
const char* WIFI_PASS  = "00979625824";

// ========================= CERTIFICADO CA MQTT =========================
static const char MQTT_MOSQ_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

// ========================= OBJETOS GLOBALES =========================
WebServer server(80);
WiFiClientSecure secureClient;
PubSubClient mqttClient(secureClient);
Servo radarServo;

// ===== Buffer de puntos para la visualización del radar en la web =====
struct MapPoint {
  int   angle;
  float distance;  // <0 => sin lectura
};

const int MAP_MAX_POINTS = 90;   // últimos 90 puntos (~barrido completo)
MapPoint mapPoints[MAP_MAX_POINTS];
int mapHead  = 0;   // índice de escritura
int mapCount = 0;   // cuántos puntos válidos hay

void storeMapPoint(int angle, float dist) {
  mapPoints[mapHead].angle    = angle;
  mapPoints[mapHead].distance = dist;
  mapHead = (mapHead + 1) % MAP_MAX_POINTS;
  if (mapCount < MAP_MAX_POINTS) mapCount++;
}


// Radar
int radarAngle      = 20;
int radarStep       = 5;
bool radarForward   = true;
unsigned long lastRadarMs = 0;

unsigned long lastMqttAttempt = 0;

// ========================= FUNCIONES HARDWARE =========================
void setupPins() {
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(EnableA, OUTPUT);
  pinMode(EnableB, OUTPUT);

  pinMode(LED_FrontLeft,  OUTPUT);
  pinMode(LED_FrontRight, OUTPUT);
  pinMode(LED_BackLeft,   OUTPUT);
  pinMode(LED_BackRight,  OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  radarServo.attach(SERVO_PIN);
  radarServo.write(radarAngle);
}

void setLeds(bool frontOn, bool backOn) {
  digitalWrite(LED_FrontLeft,  frontOn ? HIGH : LOW);
  digitalWrite(LED_FrontRight, frontOn ? HIGH : LOW);
  digitalWrite(LED_BackLeft,   backOn  ? HIGH : LOW);
  digitalWrite(LED_BackRight,  backOn  ? HIGH : LOW);
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  analogWrite(EnableA, 0);
  analogWrite(EnableB, 0);

  setLeds(false, false);
}

// Adelante: lo que antes hacía turnLeft
void moveForward(uint8_t speed) {
  // Antes en turnLeft:
  // rueda derecha adelante, izquierda atrás
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  analogWrite(EnableA, speed);
  analogWrite(EnableB, speed);
  setLeds(true, true);

}

// Atrás: lo que antes hacía turnRight
void moveBackward(uint8_t speed) {
  // Antes en turnRight:
  // rueda derecha atrás, izquierda adelante
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(EnableA, speed);
  analogWrite(EnableB, speed);
  setLeds(true, true);
}

// Izquierda: lo que antes hacía moveForward
void turnLeft(uint8_t speed) {
  // Antes en moveForward:
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  analogWrite(EnableA, speed);
  analogWrite(EnableB, speed);
  setLeds(false, true);

}

// Derecha: lo que antes hacía moveBackward
void turnRight(uint8_t speed) {
  // Antes en moveBackward:
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(EnableA, speed);
  analogWrite(EnableB, speed);
  setLeds(true, false);
}


// ========================= ULTRASONIDO + RADAR =========================
float readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return -1.0;
  return duration / 58.0;
}

void publishDistance(float dist) {
  if (!mqttClient.connected()) return;
  char payload[64];
  if (dist < 0)
    snprintf(payload, sizeof(payload), "{\"distance_cm\":null}");
  else
    snprintf(payload, sizeof(payload), "{\"distance_cm\":%.2f}", dist);

  mqttClient.publish(MQTT_TOPIC_DIST, payload);
}

void publishMapPoint(int angle, float dist) {
  // 1) Guardar SIEMPRE para el radar web
  storeMapPoint(angle, dist);

  // 2) Publicar por MQTT (si está conectado)
  if (!mqttClient.connected()) return;

  char payload[96];
  if (dist < 0) {
    snprintf(payload, sizeof(payload),
             "{\"angle_deg\":%d,\"distance_cm\":null}", angle);
  } else {
    snprintf(payload, sizeof(payload),
             "{\"angle_deg\":%d,\"distance_cm\":%.2f}", angle, dist);
  }
  mqttClient.publish(MQTT_TOPIC_MAP, payload);
}


void radarUpdate() {
  unsigned long now = millis();
  if (now - lastRadarMs < RADAR_DELAY) return;
  lastRadarMs = now;

  radarServo.write(radarAngle);
  delay(15);

  float d = readDistanceCm();
  publishDistance(d);
  publishMapPoint(radarAngle, d);

  if (radarForward) {
    radarAngle += radarStep;
    if (radarAngle >= 160) {
      radarAngle = 160;
      radarForward = false;
    }
  } else {
    radarAngle -= radarStep;
    if (radarAngle <= 20) {
      radarAngle = 20;
      radarForward = true;
    }
  }
}

// ========================= WIFI =========================
void setupWiFi() {
  Serial.println("[WiFi] Conectando a WiFi (STA)...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 30) {
    delay(500);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WiFi] Conectado como STA");
    Serial.print("[WiFi] IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n[WiFi] ERROR: no se pudo conectar a la red STA.");
    Serial.println("[WiFi] Revisa SSID/clave o que la red sea 2.4 GHz.");
  }
}


// ========================= MQTT =========================
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("MQTT msg [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void setupMQTT() {
  // ⚠ TLS cifrado pero sin validar certificado (para evitar problemas de hora/CA)
  secureClient.setInsecure();

  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
  Serial.println("[MQTT] Configurado broker test.mosquitto.org:8883 (TLS sin verificación)");
}


void reconnectMQTT() {
  if (mqttClient.connected()) return;

  unsigned long now = millis();
  if (now - lastMqttAttempt < MQTT_RETRY_MS) return;
  lastMqttAttempt = now;

  Serial.print("Conectando a MQTT...");
  String clientId = "CarroESP32-";
  clientId += String(random(0xffff), HEX);

  if (mqttClient.connect(clientId.c_str())) {
    Serial.println(" conectado");
  } else {
    Serial.print(" fallo, rc=");
    Serial.println(mqttClient.state());
  }
}

// ========================= HTTP =========================
void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleHealthCheck() {
  server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void handleMove() {
  String dir   = server.arg("dir");
  String sSpeed = server.arg("speed");

  uint8_t speed = 200;
  if (sSpeed.length() > 0) {
    int sp = sSpeed.toInt();
    speed = constrain(sp, 0, 255);
  }

  if      (dir == "forward")  moveForward(speed);
  else if (dir == "backward") moveBackward(speed);
  else if (dir == "left")     turnLeft(speed);
  else if (dir == "right")    turnRight(speed);
  else                        stopMotors();

  String json =
    "{\"direction\":\"" + dir + "\",\"speed\":" + String(speed) + "}";
  server.send(200, "application/json", json);
}

// Devuelve los últimos puntos del radar en JSON
void handleRadar() {
  String json = "[";

  for (int i = 0; i < mapCount; i++) {
    int idx = (mapHead - mapCount + i + MAP_MAX_POINTS) % MAP_MAX_POINTS;
    MapPoint &p = mapPoints[idx];

    json += "{\"angle_deg\":";
    json += p.angle;
    json += ",\"distance_cm\":";

    if (p.distance < 0) {
      json += "null";
    } else {
      json += String(p.distance, 2);
    }
    json += "}";

    if (i < mapCount - 1) json += ",";
  }

  json += "]";
  server.send(200, "application/json", json);
}



void setup() {
  Serial.begin(115200);
  delay(1000);

  setupPins();
  stopMotors();

  setupWiFi();
  setupMQTT();

  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/v1/healthcheck", HTTP_GET, handleHealthCheck);
  server.on("/api/v1/move", HTTP_GET, handleMove);
  server.on("/api/v1/move", HTTP_POST, handleMove);
  server.on("/api/v1/radar", HTTP_GET, handleRadar);

  server.begin();
  Serial.println("Servidor HTTP iniciado");
}

void loop() {
  // 1) Atender peticiones HTTP (HTML + API REST)
  server.handleClient();

  // 2) MQTT sólo si hay WiFi
  if (WiFi.status() == WL_CONNECTED) {
    if (!mqttClient.connected()) {
      reconnectMQTT();
    } else {
      mqttClient.loop();
    }
  } else {
    static unsigned long lastNoWiFi = 0;
    if (millis() - lastNoWiFi > 5000) {
      Serial.println("[MQTT] No hay WiFi, saltando MQTT.");
      lastNoWiFi = millis();
    }
  }

  // 3) Actualizar radar (servo + ultrasonido + publish MQTT)
  radarUpdate();
}

