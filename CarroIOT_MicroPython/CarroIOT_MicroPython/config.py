# config.py — pines, topicos y ajustes de red para MicroPython en ESP32

# ======== WIFI ========
WIFI_SSID = "BAZZANIR"          # cambia por tu SSID
WIFI_PASS = "00979625824"       # cambia por tu clave

# ======== MQTT (TLS) ========
MQTT_BROKER = "test.mosquitto.org"
MQTT_PORT   = 8883              # 8883=TLS, 1883=sin TLS
MQTT_CLIENT_ID = "CarroIOT-ESP32"
MQTT_TOPIC_DIST = "carro/distancia"
MQTT_TOPIC_MAP  = "carro/mapa"
MQTT_TOPIC_MOVE = "carro/move"
MQTT_RETRY_MS   = 5000

# Si tu firmware soporta validación de CA, dejamos ssl_params preparados.
USE_TLS = True
SSL_PARAMS = {}  # algunos firmwares aceptan: {"server_hostname": MQTT_BROKER}

# ======== PINES (ajusta si tu cableado es distinto) ========
# Servo (radar)
SERVO_PIN = 4

# Driver L298N
EnableA = 14   # PWM Derecho
EnableB = 12   # PWM Izquierdo
IN1 = 25       # Derecho
IN2 = 26       # Derecho
IN3 = 18       # Izquierdo
IN4 = 19       # Izquierdo

# LEDs (si no usas, puedes ignorarlos)
LED_FrontLeft  = 22
LED_FrontRight = 32
LED_BackLeft   = 23
LED_BackRight  = 33

# Sensor ultrasonido (HC-SR04)
TRIG_PIN = 27
ECHO_PIN = 5

# PWM (duty base 0-1023 en MicroPython ESP32; ajustaremos helper)
PWM_FREQ = 1000
PWM_MAX  = 1023

# Radar (barrido)
RADAR_MIN = 20
RADAR_MAX = 160
RADAR_STEP = 5
RADAR_INTERVAL_MS = 120  # tiempo entre pasos
