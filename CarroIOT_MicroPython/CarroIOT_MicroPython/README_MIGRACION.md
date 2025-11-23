# Migración a MicroPython (ESP32) — CarroIOT

**Objetivo:** portar el proyecto Arduino (ESP32) a MicroPython **sin cambiar la lógica funcional**:
- Conexión WiFi
- Servidor web (HTTP) para UI y endpoints
- Control de motores L298N, LEDs y servo (radar)
- Sensor de ultrasonido para distancia y mapeo
- Publicación MQTT a `test.mosquitto.org` (TLS 8883) en los tópicos:
  - `carro/distancia`
  - `carro/mapa`
  - `carro/move`

---
## Paso a paso

1) **Flashea MicroPython al ESP32**
   - Descarga firmware estable para ESP32 (GENÉRICO) desde https://micropython.org/download/ESP32/
   - Borra el flash y flashea (ejemplo en Windows con esptool):
     ```bash
     esptool.py --chip esp32 erase_flash
     esptool.py --chip esp32 --baud 460800 write_flash -z 0x1000 ESP32_GENERIC-*.bin
     ```
   - En **Thonny**: `Herramientas → Opciones → Intérprete` → *MicroPython (ESP32)* y selecciona el puerto.

2) **Sube los archivos al ESP32**
   - Copia los ficheros a la raíz del dispositivo:
     - `boot.py`, `main.py`, `config.py`, `web_ui.html`, `certs/isrgrootx1.pem`
   - Puedes usar **Thonny** (botón *Subir*), **mpremote** o **ampy**.
     Con `mpremote` (ejemplo):
     ```bash
     mpremote connect auto            fs cp boot.py :            fs cp main.py :            fs cp config.py :            fs cp web_ui.html :            fs mkdir certs ; fs cp certs/isrgrootx1.pem :/certs
     ```

3) **Edita credenciales y pines en `config.py`**
   - Cambia `WIFI_SSID` y `WIFI_PASS`.
   - Ajusta pines si tu cableado difiere.

4) **Prueba**
   - Reinicia el ESP32. En el **Shell** verás la IP.
   - Abre `http://<IP_DEL_ESP32>/` para la interfaz.
   - Verás `/api/move`, `/api/stop`, `/api/speed`, `/api/state` usadas por la UI.

5) **MQTT con TLS**
   - El cliente usa `ssl=True` hacia `test.mosquitto.org:8883`.
   - **Según el firmware** de MicroPython, la **validación de CA** puede variar.
     - Si falla la conexión TLS, prueba:
       - Cambiar a **1883** y `USE_TLS=False` temporalmente para depurar.
       - O actualizar a un firmware que soporte validación de CA.

---
## ¿Qué “ID” escoger y cómo subir el código?

- En **Arduino IDE** el *Board ID* típico es **“ESP32 Dev Module”**. *Pero como migramos a MicroPython, ya no compilas con Arduino IDE*.
- En **Thonny**, selecciona **Intérprete: “MicroPython (ESP32)”** y el puerto COM/tty correspondiente.
- Métodos de subida:
  - **Thonny**: Abrir cada archivo y `Guardar en el dispositivo`.
  - **mpremote**: `mpremote connect auto fs cp archivo :` (ver pasos arriba).
  - **WebREPL** (opcional si habilitas WebREPL).

---
## Equivalencias de la lógica original

- **Tópicos MQTT**: `carro/distancia`, `carro/mapa`, `carro/move` (iguales).
- **Barrido del servo**: de `RADAR_MIN` a `RADAR_MAX` y vuelta, publicando ángulo y distancia.
- **UI**: se sirve como `web_ui.html` desde el ESP32 (misma UI que `web_ui.h` original).
- **Control**: `/api/move` acepta `forward/back/left/right/stop`; `/api/speed` ajusta 0–100%.

---
## Notas
- Si tu servo necesita otro rango de `duty`, ajusta `servo_write_deg()` en `main.py`.
- El PWM en ESP32 MicroPython usa `0–1023` por defecto; mapeamos 0–100%.
- Si tu sensor HC‑SR04 no responde, revisa `TRIG_PIN`/`ECHO_PIN` y la alimentación.
