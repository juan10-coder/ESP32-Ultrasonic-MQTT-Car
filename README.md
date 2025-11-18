# Control y Simulación de Sensor Ultrasonido con ESP32 + MQTT

Este proyecto implementa un sistema IoT con ESP32 capaz de controlar el movimiento de un vehículo mediante HTTP y MQTT, además de simular lecturas de un sensor ultrasónico (HC-SR04) que se publican periódicamente a un tópico MQTT distinto.

El sistema combina comunicación en red, procesamiento local, simulación de sensores y publicación en la nube, manteniendo buenas prácticas de modularización con archivos `.h` para configuraciones y variables de preprocesador.

---

## Abstract

En este proyecto se diseña e implementa una arquitectura IoT para un carro controlado por ESP32.  
El sistema:
- Acepta comandos HTTP para moverse en distintas direcciones (adelante, atrás, izquierda, derecha).  
- Publica cada acción en un tópico MQTT de control.  
- Simula lecturas de distancia mediante un mock del sensor ultrasónico HC-SR04, que genera valores aleatorios realistas y los envía a un tópico MQTT distinto cada cierto tiempo.  

La implementación se desarrolló bajo principios de ingeniería modular, empleando un archivo `config.h` para definir variables de preprocesador y facilitar mantenimiento y portabilidad.

**Palabras clave:** IoT, ESP32, MQTT, HC-SR04, Arduino, Simulación, Ingeniería IEEE.

---

## Requisitos del sistema

- Microcontrolador: ESP32  
- Entorno: Arduino IDE o PlatformIO  
- Librerías necesarias:  
  - WiFi.h  
  - WebServer.h  
  - PubSubClient.h  
- Broker MQTT: Mosquitto (o test.mosquitto.org para pruebas)  
- Cliente de pruebas: Postman o MQTT Explorer  
- Conexión serial: 115200 bps  

---

## Funcionalidades principales

- Control remoto de movimiento vía HTTP  
- Publicación de eventos de movimiento en MQTT (`carro/movimiento`)  
- Simulación (mock) del sensor ultrasónico HC-SR04  
- Publicación periódica de la distancia simulada en MQTT (`carro/distancia`)  
- Uso de variables de preprocesador centralizadas en `config.h`  
- Reconexión WiFi y MQTT con retardo exponencial y jitter  
- Código modular, documentado y escalable  

---

## Arquitectura general

**[Espacio para diagrama de arquitectura del sistema (bloques ESP32 - MQTT - Cliente)]**

---

## Descripción técnica

El proyecto consta de dos archivos principales:

### 1. `main.ino`
Contiene la lógica de conexión WiFi, endpoints HTTP, control de motores, simulación del sensor y publicación periódica a MQTT.

### 2. `config.h`
Define las variables de preprocesador que configuran:
- Pines del ESP32 (motores, LEDs, sensor)
- Credenciales WiFi
- Configuración MQTT (broker, tópicos, ID de cliente)

**[Espacio para captura del código o estructura de carpetas del repositorio]**

---

## Endpoints HTTP implementados

| Método | URL | Descripción | Parámetros | Ejemplo |
|--------|-----|--------------|-------------|----------|
| GET | `/status` | Verifica el estado del servidor | — | `{ "status":"Servidor operativo" }` |
| POST | `/move` | Envía comando de movimiento | `direccion`, `velocidad`, `duracion` | `http://<ip>/move?direccion=adelante&velocidad=200&duracion=3` |

**[Espacio para captura Postman (endpoint /move)]**

---

## Tópicos MQTT

| Tópico | Descripción | Ejemplo JSON publicado |
|--------|--------------|-------------------------|
| `carro/movimiento` | Envía los datos del movimiento ejecutado | `{ "cliente":"192.168.1.12", "direccion":"adelante", "velocidad":200, "duracion":3 }` |
| `carro/distancia` | Publica la distancia simulada del sensor mock | `{ "distancia_cm":145.32 }` |

**[Espacio para captura MQTT Explorer con ambos tópicos]**

---

## Pruebas realizadas

**[Captura 1: Serial Monitor mostrando conexión WiFi y MQTT]**  
**[Captura 2: Postman ejecutando /move]**  
**[Captura 3: Mensajes MQTT en carro/movimiento y carro/distancia]**  
**[Captura 4: Simulación de lecturas del sensor mock cada 5 segundos]**

---

## Instrucciones de uso

1. Configurar credenciales WiFi y el broker MQTT en `config.h`.  
2. Cargar el código al ESP32 mediante Arduino IDE.  
3. Conectar el ESP32 y abrir el Monitor Serial (115200 bps).  
4. Verificar la conexión WiFi y la dirección IP asignada.  
5. Probar los endpoints HTTP (`/status`, `/move`).  
6. Abrir el cliente MQTT y suscribirse a:  
   - `carro/movimiento`  
   - `carro/distancia`  
7. Observar la publicación automática del mock ultrasónico cada 5 segundos.  

---

## Diagramas del sistema

### **Diagrama de secuencia UML**  
![Secuencia UML](DiagramaSecuencial.png)


### **Diagrama de flujo**
![Flujo UML](image.png)

---

# Pruebas de Conexión MQTT con TLS en ESP32

Este documento describe las pruebas realizadas para conectar un ESP32 a
un broker MQTT utilizando diferentes configuraciones de seguridad
(puerto inseguro, TLS sin validación, TLS con validación y TLS con
certificado CA).

Incluye código, evidencias de monitor serial y conclusiones de seguridad
para cada escenario.

------------------------------------------------------------------------

## Preparación del Entorno

### Modificaciones necesarias en `config.h`

``` cpp
// Puerto seguro MQTT
#define MQTT_PORT_SECURE 8883
#define MQTT_PORT_INSECURE 1883
```

------------------------------------------------------------------------

#  Pruebas de Código

------------------------------------------------------------------------

##  Prueba 1 --- Conexión al puerto seguro **sin TLS**

**Objetivo:** Cambiar solo el puerto a `8883` sin usar TLS, para
verificar qué ocurre.

### Fragmento de código

``` cpp
// PRUEBA 1: Cambiar solo el puerto a 8883
// Resultado esperado: FALLO - El broker requiere TLS

void setup() {
  Serial.begin(115200);
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  pinMode(IN3_PIN, OUTPUT);
}
```

###  Evidencia - Monitor Serial

    WiFi conectado
    IP: 192.168.1.100
    Intentando conectar a puerto seguro sin TLS...
    Error: No se puede conectar al broker MQTT
    Reintentando conexión MQTT...
    [Bucle infinito de reintentos]

###  Conclusión

 **FALLA.** El puerto `8883` exige TLS, pero se está usando
`WiFiClient`, que no soporta cifrado.

------------------------------------------------------------------------

##  Prueba 2 --- TLS sin validación de certificado (`setInsecure()`)

**Objetivo:** Usar `WiFiClientSecure` sin validar certificados.

### Código simplificado

``` cpp
#include <WiFi.h>
#include <WebServer.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "config.h"

WiFiClientSecure espClient;   // Cliente TLS sin validación
```

###  Evidencia - Monitor Serial

    === PRUEBA 2: TLS SIN VALIDACIÓN DE CERTIFICADO ===
    ⚠️ Validación de certificados DESHABILITADA
    Intentando conexión MQTT TLS (sin validación)... ✓ Conectado!
    Distancia publicada vía TLS: 145.67 cm
    Distancia publicada vía TLS: 78.23 cm

###  Conclusión

✔ **FUNCIONA**, pero:

 **Vulnerable a ataques Man-in-the-Middle (MitM)**\
Los datos viajan cifrados pero no se verifica el servidor.

------------------------------------------------------------------------

##  Prueba 3 --- TLS con validación, **pero sin cargar certificado CA**

**Objetivo:** Activar validación de certificado sin proporcionar CA →
debe fallar.

### Código simplificado

``` cpp
WiFiClientSecure espClient;
PubSubClient client(espClient);
```

###  Evidencia - Monitor Serial

    === PRUEBA 3: TLS CON VALIDACIÓN (SIN CERTIFICADO) ===
    ✓ Validación habilitada
    ✗ Certificado CA NO proporcionado

    Intento 1... rc=-2 TLS Error: -1
    → No se pudo verificar el certificado del servidor
    Reintentando...

    Intento 2... rc=-2 TLS Error: -1
    Reintentando...

###  Conclusión

❌ **FALLA**, como se esperaba.\
El ESP32 no puede validar el certificado sin un CA cargado.

------------------------------------------------------------------------

##  Prueba 4 --- TLS con certificado CA **válido**

**Objetivo:** Proveer el certificado correcto del broker y validar la
conexión segura.

### Obtención del certificado CA

``` bash
openssl s_client -showcerts -connect test.mosquitto.org:8883 < /dev/null   | openssl x509 -outform PEM > mosquitto_ca.pem
```

###  Evidencia - Monitor Serial

    === PRUEBA 4: TLS CON CERTIFICADO VÁLIDO ===
    ✓ Cargando certificado CA...
    ✓ Certificado validado correctamente
    ✓ Conexión MQTT segura y cifrada establecida

     Distancia publicada de forma SEGURA: 123.45 cm

###  Conclusión

✔ **ÉXITO TOTAL.**\
TLS habilitado + validación correcta → comunicación *segura y protegida*
contra ataques MitM.

------------------------------------------------------------------------

#  Resumen de Resultados

  ------------------------------------------------------------------------------------
  Prueba    Cliente              Puerto    Certificado     Resultado     Seguridad
  --------- -------------------- --------- --------------- ------------- -------------
  1         WiFiClient           8883      No              ❌ Falla      N/A

  2         WiFiClientSecure +   8883      No              ✔ Funciona    ⚠ Vulnerable
            setInsecure()                                                MitM

  3         WiFiClientSecure     8883      No              ❌ Falla      N/A
            (validación)                                                 

  4         WiFiClientSecure +   8883      Sí              ✔ Funciona    ✔ Seguro
            setCACert()                                                  
  ------------------------------------------------------------------------------------

------------------------------------------------------------------------

#  Archivos recomendados

### `certificates.h`

``` cpp
#ifndef CERTIFICATES_H
#define CERTIFICATES_H

// CERTIFICADO CA PARA test.mosquitto.org
// (Contenido PEM aquí)

#endif
```

------------------------------------------------------------------------

#  Recomendaciones Finales para Producción

-   Nunca usar `setInsecure()` en dispositivos reales\
-   Implementar actualización OTA para renovar certificados\
-   Monitorear fechas de expiración proactivamente\
-   Considerar autenticación mutua (mTLS) para mayor seguridad\
-   Almacenar certificados en filesystem para facilitar actualizaciones\
-   Usar Let's Encrypt para certificados automáticos si el broker es
    propio

------------------------------------------------------------------------

#  Estructura del Proyecto

    proyecto/
    ├── config.h          // Configuración y certificados
    └── main.ino          // Código principal con TLS
    
------------------------------------------------------------------------

#  Script para renovación de certificados

``` bash
#!/bin/bash
BROKER="test.mosquitto.org"
PORT="8883"
OUTPUT="mosquitto_ca.pem"

echo "Obteniendo certificado..."
openssl s_client -showcerts -connect $BROKER:$PORT < /dev/null   | openssl x509 -outform PEM > $OUTPUT

echo "Certificado guardado en $OUTPUT"
```


## Referencias

[1] Espressif Systems – ESP32 Wi-Fi Programming Guide, 2023.  
[2] Arduino – PubSubClient & WebServer Libraries Documentation, 2024.  
[3] IEEE – Standards for IoT System Documentation, 2020.  

---

## Autores

**Juan David Henao Osorio** – Universidad de La Sabana  
**Santiago Bazzani Rincón** – Universidad de La Sabana

