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

#endif // CONFIG_H