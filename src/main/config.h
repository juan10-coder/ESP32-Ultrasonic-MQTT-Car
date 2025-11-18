#ifndef CONFIG_H
#define CONFIG_H

// ==============================
// PINES DE MOTORES (L298N u otro driver)
// ==============================
#define IN1_PIN 25
#define IN2_PIN 26
#define IN3_PIN 18
#define IN4_PIN 19

// ==============================
// LEDS DECORATIVOS / DIRECCIONALES
// ==============================
#define LED_FRONT_LEFT   22
#define LED_FRONT_RIGHT  32
#define LED_BACK_LEFT    23
#define LED_BACK_RIGHT   33

// ==============================
// PINES SENSOR ULTRASÓNICO
// (Si luego conectas el físico)
// ==============================
#ifndef TRIG_PIN
#define TRIG_PIN 35  // OUTPUT
#endif

#ifndef ECHO_PIN
#define ECHO_PIN 34  // INPUT (→ usar divisor a 3.3V en ESP32)
#endif

// ==============================
// WiFi
// ==============================
#define WIFI_SSID "TU_SSID"
#define WIFI_PASS "TU_PASSWORD"

// ==============================
// MQTT
// ==============================
#define MQTT_SERVER     "test.mosquitto.org" // o tu broker
#define MQTT_PORT       1883
#define MQTT_CLIENT_ID  "ESP32Carro"         // cambia si tienes varios
#define MQTT_TOPIC_MOV  "carro/movimiento"
#define MQTT_TOPIC_DIST "carro/distancia"


// ============================================
// CERTIFICADO CA PARA test.mosquitto.org
// ============================================
// Válido desde: Jun 9 11:06:39 2020 GMT
// Válido hasta: Jun 7 11:06:39 2030 GMT
// Emisor: Mosquitto CA
// Sujeto: CN=mosquitto.org
// ============================================
// Obtenido con:
// openssl s_client -showcerts -connect test.mosquitto.org:8883 < /dev/null 2>/dev/null | openssl x509 -outform PEM
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

// ============================================
// Función auxiliar para verificar expiración
// ============================================
void checkCertificateExpiration() {
  // El certificado expira el: Jun 7 11:06:39 2030 GMT
  // Recordar actualizar antes de esta fecha
  
  Serial.println("Información del certificado:");
  Serial.println("  Emisor: Mosquitto CA");
  Serial.println("  Dominio: mosquitto.org");
  Serial.println("  Válido desde: 2020-06-09");
  Serial.println("  Expira: 2030-06-07");
  Serial.println("  ⚠️  Recordar actualizar antes de la expiración");
}

// ============================================
// NOTAS IMPORTANTES:
// ============================================
// 1. Este certificado es específico para test.mosquitto.org
// 2. Si cambias de broker, necesitarás obtener su certificado
// 3. Para obtener un certificado de otro servidor:
//    openssl s_client -showcerts -connect SERVIDOR:8883 < /dev/null 2>/dev/null | openssl x509 -outform PEM
// 4. Para verificar un certificado:
//    openssl x509 -in certificate.pem -text -noout
// 5. Para producción, considera almacenar certificados en SPIFFS/LittleFS
//    para facilitar actualizaciones OTA


#endif // CONFIG_H
