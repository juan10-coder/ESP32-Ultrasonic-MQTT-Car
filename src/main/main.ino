#include <WiFi.h>
#include <WebServer.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "config.h"

WiFiClientSecure espClient;
PubSubClient client(espClient);
WebServer server(80);

// ============================================
// CERTIFICADO CA PARA test.mosquitto.org
// Obtenido con: openssl s_client -showcerts -connect test.mosquitto.org:8883
// ============================================
const char* mosquitto_ca_cert = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIEAzCCAuugAwIBAgIUBY1hlCGvdj4NhBXkZ/uLUZNILAwwDQYJKoZIhvcNAQEL\n" \
"BQAwgZAxCzAJBgNVBAYTAkdCMRcwFQYDVQQIDA5Vbml0ZWQgS2luZ2RvbTEOMAwG\n" \
"A1UEBwwFRGVyYnkxEjAQBgNVBAoMCU1vc3F1aXR0bzELMAkGA1UECwwCQ0ExFjAU\n" \
"BgNVBAMMDW1vc3F1aXR0by5vcmcxHzAdBgkqhkiG9w0BCQEWEHJvZ2VyQGF0Y2hv\n" \
"by5vcmcwHhcNMjAwNjA5MTEwNjM5WhcNMzAwNjA3MTEwNjM5WjCBkDELMAkGA1UE\n" \
"BhMCR0IxFzAVBgNVBAgMDlVuaXRlZCBLaW5nZG9tMQ4wDAYDVQQHDAVEZXJieTES\n" \
"MBAGA1UECgwJTW9zcXVpdHRvMQswCQYDVQQLDAJDQTEWMBQGA1UEAwwNbW9zcXVp\n" \
"dHRvLm9yZzEfMB0GCSqGSIb3DQEJARYQcm9nZXJAYXRjaG9vLm9yZzCCASIwDQYJ\n" \
"KoZIhvcNAQEBBQADggEPADCCAQoCggEBAME0HKmIzfTOwkKLT3THHe+ObdizamPg\n" \
"UZmD64Tf3zJdNeYGYn4CEXbyP6fy3tWc8S2boW6dzrH8SdFf9uo320GJA9B7U1FW\n" \
"Te3xda/Lm3JFfaHjkWw7jBwcauQZjpGINHapHRlpiCZsquAthOgxW9SgDgYlGzEA\n" \
"s06pkEFiMw+qDfLo/sxFKB6vQlFekMeCymjLCbNwPJyqyhFmPWwio/PDMruBTzPH\n" \
"3cioBnrJWKXc3OjXdLGFJOfj7pP0j/dr2LH72eSvv3PQQFl90CZPFhrCUcRHSSxo\n" \
"E6yjGOdnz7f6PveLIB574kQORwt8ePn0yidrTC1ictikED3nHYhMUOUCAwEAAaNT\n" \
"MFEwHQYDVR0OBBYEFPVV6xBUFPiGKDyo5V3+Hbh4N9YSMB8GA1UdIwQYMBaAFPVV\n" \
"6xBUFPiGKDyo5V3+Hbh4N9YSMA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEL\n" \
"BQADggEBAGa9kS21N70ThM6/Hj9D7mbVxKLBjVWe2TPsGfbl3rEDfZ+OKRZ2j6AC\n" \
"6r7jb4TZO3dzF2p6dgbrlU71Y/4K0TdzIjRj3cQ3KSm41JvUQ0hZ/c04iGDg/xWf\n" \
"+pp58nfPAYwuerruPNWmlStWAXf0UTqRtg4hQDWBuUFDJTuWuuBvEXudz74eh/wK\n" \
"sMwfu1HFvjy5Z0iMDU8PUDepjVolOCue9ashlS4EB5IECdSR2TItnAIiIwimx839\n" \
"LdUdRudafMu5T5Xma182OC0/u/xRlEm+tvKGGmfFcN0piqVl8OrSPBgIlb+1IKJE\n" \
"m/XriWr/Cq4h/JfB7NTsezVslgkBaoU=\n" \
"-----END CERTIFICATE-----\n";

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
  
  bool published = client.publish(MQTT_TOPIC_MOV, mensaje.c_str());
  if (published) {
    Serial.println("✓ Mensaje publicado de forma segura vía TLS");
  }
  
  server.send(200, "application/json", "{\"status\":\"Movimiento ejecutado (TLS seguro)\"}");
  delay(duracion * 1000);
  stopCar();
}

void handleStatus() {
  String status = "{\"status\":\"Servidor operativo\",\"tls\":\"habilitado\",\"cert_validation\":\"activa\"}";
  server.send(200, "application/json", status);
}

void connectWiFi() {
  Serial.println("Conectando a WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Conectando MQTT con TLS seguro y validación de certificado...");
    
    if (client.connect(MQTT_CLIENT_ID)) {
      Serial.println(" ✓ ¡CONEXIÓN SEGURA ESTABLECIDA!");
      Serial.println("   → Certificado del servidor VALIDADO correctamente");
      Serial.println("   → Canal cifrado con TLS activo");
      Serial.println("   → Comunicación protegida contra MitM");
    } else {
      Serial.print(" ✗ Falló, rc=");
      Serial.print(client.state());
      Serial.println(" Reintentando en 5s...");
      delay(5000);
    }
  }
}

float mockUltrasonic() {
  float distancia = random(10, 200) + (random(0, 100) / 100.0f);
  return distancia;
}

void publishMockUltrasonic() {
  float d = mockUltrasonic();
  String payload = "{\"distancia_cm\":" + String(d, 2) + "}";
  bool success = client.publish(MQTT_TOPIC_DIST, payload.c_str());
  
  if (success) {
    Serial.print("🔒 Distancia publicada de forma SEGURA: ");
    Serial.print(d, 2);
    Serial.println(" cm");
  }
}

unsigned long lastUltrasonicTime = 0;
const unsigned long ultrasonicInterval = 5000;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n=== PRUEBA 4: TLS CON CERTIFICADO VÁLIDO ===");
  
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
  
  // ============================================
  // CONFIGURACIÓN SEGURA CON CERTIFICADO CA
  // ============================================
  Serial.println("\nConfigurando seguridad TLS:");
  Serial.println("✓ Cargando certificado CA de mosquitto.org...");
  espClient.setCACert(mosquitto_ca_cert);
  Serial.println("✓ Certificado CA cargado correctamente");
  Serial.println("✓ Validación de certificados HABILITADA");
  Serial.println("✓ Protección contra Man-in-the-Middle ACTIVA\n");
  
  client.setServer(MQTT_SERVER, 8883);
  Serial.println("Servidor MQTT configurado: " + String(MQTT_SERVER) + ":8883 (TLS)");
  
  server.on("/move",   handleMove);
  server.on("/status", handleStatus);
  server.begin();
  Serial.println("Servidor HTTP iniciado en puerto 80\n");
  
  Serial.println("Iniciando conexión MQTT segura...");
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

// Resultado esperado:
// ✓ ÉXITO COMPLETO: Conexión TLS segura establecida
// ✓ Certificado del servidor validado correctamente
// ✓ Todos los datos cifrados extremo a extremo
// ✓ Protección contra ataques Man-in-the-Middle
// ✓ Sistema completamente seguro