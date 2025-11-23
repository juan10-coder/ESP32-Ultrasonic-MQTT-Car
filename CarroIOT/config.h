#ifndef CONFIG_H
#define CONFIG_H

// =======================================================
// ===============  PINES DEL ROBOT (ESP32) ==============
// =======================================================

// ----------- Servo Radar -----------
#define SERVO_PIN 4    // Microservo SG90 para barrido tipo radar

// ----------- Motores L298N ----------
#define EnableA 14     // PWM Motor Derecho
#define EnableB 12     // PWM Motor Izquierdo

#define IN1 25         // L298N IN1 (Motor Derecho)
#define IN2 26         // L298N IN2 (Motor Derecho)
#define IN3 18         // L298N IN3 (Motor Izquierdo)
#define IN4 19         // L298N IN4 (Motor Izquierdo)

// ----------- LEDs -----------
#define LED_FrontLeft   22
#define LED_FrontRight  32
#define LED_BackLeft    23
#define LED_BackRight   33

// ----------- Sensor Ultrasonido HC-SR04 -----------
#define TRIG_PIN 5
#define ECHO_PIN 27

// =======================================================
// =================== RADAR CONFIG ======================
// =======================================================
#define RADAR_DELAY 100      // tiempo entre mediciones (ms)
#define RADAR_MIN_ANGLE 20   // ángulo mínimo
#define RADAR_MAX_ANGLE 160  // ángulo máximo
#define RADAR_STEP 5         // grados por movimiento

// =======================================================
// =================== PWM CONFIG ========================
// =======================================================
#define PWM_FREQ 1000              // Frecuencia PWM
#define PWM_RESOLUTION 8           // 8 bits (0–255 duty)

// =======================================================
// ===================== WIFI AP =========================
// =======================================================
#define AP_SSID      "Equipo3-ESP32"
#define AP_PASS      "123456789"
#define AP_CH        1
#define AP_MAX_CONN  4

// =======================================================
// ====================== MQTT ===========================
// =======================================================
#define MQTT_BROKER  "test.mosquitto.org"
#define MQTT_PORT    8883          // TLS

#define MQTT_TOPIC_DIST "carro/distancia"
#define MQTT_TOPIC_MAP  "carro/mapa"
#define MQTT_TOPIC_MOVE "carro/move"

// Tiempo entre reintentos MQTT
#define MQTT_RETRY_MS 5000

#endif
