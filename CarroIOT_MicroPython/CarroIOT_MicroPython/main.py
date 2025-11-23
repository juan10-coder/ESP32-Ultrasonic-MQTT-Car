import network, time, ujson, ubinascii, socket, _thread
import machine
from machine import Pin, PWM
try:
    import ussl as ssl
except:
    import ssl
from umqtt.robust import MQTTClient
import config as C

# ========= WIFI =========
def wifi_connect():
    sta = network.WLAN(network.STA_IF)
    if not sta.active():
        sta.active(True)
    if not sta.isconnected():
        sta.connect(C.WIFI_SSID, C.WIFI_PASS)
        t0 = time.ticks_ms()
        while not sta.isconnected() and time.ticks_diff(time.ticks_ms(), t0) < 20000:
            time.sleep_ms(200)
    return sta

# ========= MQTT =========
mqtt = None
last_mqtt_try_ms = 0

def mqtt_connect():
    global mqtt
    if mqtt:
        try:
            mqtt.disconnect()
        except:
            pass
    cid = C.MQTT_CLIENT_ID + "-" + ubinascii.hexlify(machine.unique_id()).decode()[-4:]
    mqtt = MQTTClient(client_id=cid,
                      server=C.MQTT_BROKER,
                      port=C.MQTT_PORT,
                      keepalive=30,
                      ssl=C.USE_TLS,
                      ssl_params=C.SSL_PARAMS if hasattr(C, "SSL_PARAMS") else {})
    mqtt.set_last_will(C.MQTT_TOPIC_MOVE, b'{"status":"offline"}', retain=False, qos=0)
    mqtt.connect()
    mqtt.publish(C.MQTT_TOPIC_MOVE, b'{"status":"online"}', retain=False, qos=0)
    return mqtt

def mqtt_safe_pub(topic: str, payload: dict):
    global mqtt, last_mqtt_try_ms
    data = ujson.dumps(payload).encode()
    try:
        if mqtt is None:
            mqtt_connect()
        mqtt.publish(topic, data)
    except Exception as e:
        # reintento simple
        try:
            mqtt_connect()
            mqtt.publish(topic, data)
        except:
            pass

# ========= HARDWARE =========
# Motores (L298N)
pwmA = PWM(Pin(C.EnableA), freq=C.PWM_FREQ, duty=0)
pwmB = PWM(Pin(C.EnableB), freq=C.PWM_FREQ, duty=0)
IN1 = Pin(C.IN1, Pin.OUT); IN2 = Pin(C.IN2, Pin.OUT)
IN3 = Pin(C.IN3, Pin.OUT); IN4 = Pin(C.IN4, Pin.OUT)

# LEDs
try:
    LED_FL = Pin(C.LED_FrontLeft,  Pin.OUT, value=0)
    LED_FR = Pin(C.LED_FrontRight, Pin.OUT, value=0)
    LED_BL = Pin(C.LED_BackLeft,  Pin.OUT, value=0)
    LED_BR = Pin(C.LED_BackRight, Pin.OUT, value=0)
except:
    LED_FL = LED_FR = LED_BL = LED_BR = None

# Servo (SG90) en PWM 50Hz (duty 40-115 aprox en ESP32 MicroPython)
servo = PWM(Pin(C.SERVO_PIN), freq=50, duty=0)
def servo_write_deg(deg):
    # Mapea 0-180° a duty entre ~40-115 (ajusta si tu servo lo requiere)
    duty = int(40 + (deg/180.0)*75)
    if duty < 40: duty = 40
    if duty > 115: duty = 115
    servo.duty(duty)

# Ultrasonido
TRIG = Pin(C.TRIG_PIN, Pin.OUT, value=0)
ECHO = Pin(C.ECHO_PIN, Pin.IN)
def distance_cm(timeout_us=30000):
    TRIG.off()
    time.sleep_us(3)
    TRIG.on()
    time.sleep_us(10)
    TRIG.off()
    try:
        dur = machine.time_pulse_us(ECHO, 1, timeout_us)
        if dur < 0:
            return None
        # Sonido ~343 m/s => 29.1 us por cm ida y vuelta
        return (dur / 58.2)
    except:
        return None

# Helpers motores
def _pwm(pct):
    if pct < 0: pct = 0
    if pct > 100: pct = 100
    return int((pct/100.0) * C.PWM_MAX)

def motores_stop():
    IN1.off(); IN2.off(); IN3.off(); IN4.off()
    pwmA.duty(0); pwmB.duty(0)
    if LED_FL: LED_FL.off(); LED_FR.off(); LED_BL.off(); LED_BR.off()

def motores_forward(speed=65):
    IN1.on();  IN2.off()
    IN3.on();  IN4.off()
    pwmA.duty(_pwm(speed)); pwmB.duty(_pwm(speed))
    if LED_FL: LED_FL.on(); LED_FR.on(); LED_BL.off(); LED_BR.off()

def motores_back(speed=65):
    IN1.off(); IN2.on()
    IN3.off(); IN4.on()
    pwmA.duty(_pwm(speed)); pwmB.duty(_pwm(speed))
    if LED_FL: LED_FL.off(); LED_FR.off(); LED_BL.on(); LED_BR.on()

def motores_left(speed=55):
    # derecha adelante, izquierda atrás
    IN1.on();  IN2.off()
    IN3.off(); IN4.on()
    pwmA.duty(_pwm(speed)); pwmB.duty(_pwm(speed))

def motores_right(speed=55):
    # derecha atrás, izquierda adelante
    IN1.off(); IN2.on()
    IN3.on();  IN4.off()
    pwmA.duty(_pwm(speed)); pwmB.duty(_pwm(speed))

# ========= ESTADO =========
state = {
    "speed": 65,
    "dir": "stop",
    "angle": C.RADAR_MIN,
    "last_dist_cm": None
}

# ========= RADAR SCAN THREAD =========
def radar_task():
    angle = C.RADAR_MIN
    step  = C.RADAR_STEP
    forward = True
    while True:
        servo_write_deg(angle)
        d = distance_cm()
        state["angle"] = angle
        state["last_dist_cm"] = d
        mqtt_safe_pub(C.MQTT_TOPIC_DIST, {"angle": angle, "cm": d})
        if forward:
            angle += step
            if angle >= C.RADAR_MAX:
                angle = C.RADAR_MAX
                forward = False
                mqtt_safe_pub(C.MQTT_TOPIC_MAP, {"sweep":"right->left"})
        else:
            angle -= step
            if angle <= C.RADAR_MIN:
                angle = C.RADAR_MIN
                forward = True
                mqtt_safe_pub(C.MQTT_TOPIC_MAP, {"sweep":"left->right"})
        time.sleep_ms(C.RADAR_INTERVAL_MS)

# ========= HTTP SERVER (muy simple) =========
def http_response(conn, status="200 OK", ctype="text/html; charset=utf-8", body=b""):
    conn.sendall("HTTP/1.1 %s\r\n" % status)
    conn.sendall("Content-Type: %s\r\n" % ctype)
    conn.sendall("Cache-Control: no-cache\r\n")
    conn.sendall("Connection: close\r\n\r\n")
    if body:
        if isinstance(body, str):
            body = body.encode()
        conn.sendall(body)

def handle_move(cmd):
    cmd = cmd.lower()
    if cmd == "forward":
        motores_forward(state["speed"]); state["dir"]="forward"
    elif cmd == "back":
        motores_back(state["speed"]); state["dir"]="back"
    elif cmd == "left":
        motores_left(state["speed"]); state["dir"]="left"
    elif cmd == "right":
        motores_right(state["speed"]); state["dir"]="right"
    else:
        motores_stop(); state["dir"]="stop"
    mqtt_safe_pub(C.MQTT_TOPIC_MOVE, {"move": state["dir"], "speed": state["speed"]})

def serve():
    addr = socket.getaddrinfo("0.0.0.0", 80)[0][-1]
    s = socket.socket()
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind(addr); s.listen(2)
    print("HTTP en http://%s" % network.WLAN(network.STA_IF).ifconfig()[0])
    while True:
        try:
            conn, client = s.accept()
            conn.settimeout(3.0)
            req = conn.recv(1024).decode("utf-8", "ignore")
            # Rutas muy básicas
            if req.startswith("GET / "):
                # Sirve la web
                try:
                    with open("web_ui.html", "rb") as f:
                        body = f.read()
                except:
                    body = b"<h1>web_ui.html no encontrado</h1>"
                http_response(conn, body=body)
            elif req.startswith("GET /api/state"):
                payload = ujson.dumps({"ip": network.WLAN(network.STA_IF).ifconfig()[0], **state})
                http_response(conn, ctype="application/json", body=payload)
            elif req.startswith("POST /api/move"):
                # parse body simple (cmd=...)
                if "\r\n\r\n" in req:
                    body = req.split("\r\n\r\n",1)[1]
                else:
                    body = ""
                if "forward" in body:  handle_move("forward")
                elif "back" in body:   handle_move("back")
                elif "left" in body:   handle_move("left")
                elif "right" in body:  handle_move("right")
                else:                  handle_move("stop")
                http_response(conn, ctype="application/json", body='{"ok":true}')
            elif req.startswith("POST /api/stop"):
                handle_move("stop")
                http_response(conn, ctype="application/json", body='{"ok":true}')
            elif req.startswith("POST /api/speed"):
                # body: speed=NN
                sp = state["speed"]
                if "\r\n\r\n" in req:
                    body = req.split("\r\n\r\n",1)[1]
                    try:
                        val = int(body.split("=",1)[1])
                        if 0 <= val <= 100:
                            state["speed"] = val
                    except:
                        pass
                http_response(conn, ctype="application/json", body=ujson.dumps({"speed":state["speed"]}))
            else:
                http_response(conn, status="404 Not Found", body="404")
        except Exception as e:
            try:
                conn.close()
            except:
                pass

def main():
    sta = wifi_connect()
    print("WIFI:", sta.ifconfig())
    try:
        mqtt_connect()
    except Exception as e:
        print("MQTT error (se reintentará en publicaciones):", e)
    # Hilo de radar
    _thread.start_new_thread(radar_task, ())
    # Servidor HTTP
    serve()

if __name__ == "__main__":
    main()
