## 1 ¿Qué es el protocolo TLS, cuál es su importancia y qué es un certificado en ese contexto?
**TLS (Transport Layer Security)** es un protocolo criptográfico que cifra y autentica la comunicación entre dos extremos (cliente y servidor).  
Su importancia radica en:
- **Confidencialidad:** evita que terceros lean los datos transmitidos.  
- **Integridad:** detecta alteraciones durante la transmisión.  
- **Autenticidad:** garantiza que el servidor (y opcionalmente el cliente) es quien dice ser.

Un **certificado digital** (generalmente en formato X.509) vincula una clave pública con una identidad (como un dominio o empresa) y es firmado por una **Autoridad Certificadora (CA)** para que otros puedan confiar en él.

---

## 2️ ¿A qué riesgos se está expuesto si no se usa TLS?
- **Intercepción de datos (sniffing):** robo de contraseñas, tokens o información sensible.  
- **Ataques MITM (Man-In-The-Middle):** manipulación o redirección de mensajes.  
- **Suplantación de identidad:** conexión a servidores falsos.  
- **Incumplimiento de normativas:** riesgo legal al no proteger datos personales.

---

## 3️ ¿Qué es un CA (Certificate Authority)?
Una **Autoridad Certificadora (CA)** es una entidad de confianza que emite y firma certificados digitales, validando previamente la identidad del solicitante.  
Los navegadores, sistemas operativos y microcontroladores confían en las CAs raíz preinstaladas, extendiendo esa confianza a los certificados que ellas firman.

---

## 4️ ¿Qué es una cadena de certificados y cuál es la vigencia promedio de los eslabones de la cadena?
Una **cadena de certificados** enlaza:
```

Certificado del servidor → CA intermedia → CA raíz

````
Cada eslabón debe ser validado para establecer confianza.

- Certificados de servidor (leaf): 90 días – 1 año  
- Certificados intermedios: 1 – 5 años  
- Certificados raíz: 10 – 25 años  

---

## 5️ ¿Qué es un keystore y qué es un certificate bundle?
- **Keystore:** almacén que contiene certificados y claves privadas (formatos como `.jks` o `.p12`).  
- **Certificate bundle:** archivo que agrupa varias CAs (por ejemplo, un *CA bundle* o `crt_bundle.bin` en ESP32) para validar múltiples dominios con distintas autoridades.

---

## 6️ ¿Qué es la autenticación mutua en el contexto de TLS?
La **autenticación mutua (mTLS)** ocurre cuando:
1. El cliente valida el certificado del servidor.  
2. El servidor también valida un certificado de cliente.  
De esta forma, **ambos extremos se autentican** y la conexión es más segura.

---

## 7 ¿Cómo se habilita la validación de certificados en el ESP32?

### Arduino (`WiFiClientSecure`)
```cpp
#include <WiFiClientSecure.h>
WiFiClientSecure net;

// Opción A: cargar certificado raíz específico
extern const char root_ca[] PROGMEM;
net.setCACert(root_ca);

// Opción B: usar el bundle del core ESP32
net.setCACertBundle(rootCABundlePlaceholder);
````

> **Nota:** sincroniza la hora del ESP32 mediante NTP (`configTime()`) para que la validación de fechas del certificado funcione correctamente.

### 🔹 ESP-IDF (`esp_crt_bundle`)

```c
#include "esp_crt_bundle.h"

esp_tls_cfg_t cfg = {
  .crt_bundle_attach = esp_crt_bundle_attach
};
```

---

## 8️ Si el sketch necesita conectarse a múltiples dominios con certificados generados por CAs distintos, ¿qué alternativas hay?

1. **Usar un bundle de certificados raíz** (más fácil).
2. Cargar la CA correspondiente con `setCACert()` según el dominio.
3. **Pinning de certificado o huella digital (fingerprint)**: más seguro, pero rígido.
4. `setInsecure()` → deshabilita validación (solo para pruebas).

---

## 9️ ¿Cómo se puede obtener el certificado para un dominio?

Desde la terminal:

```bash
openssl s_client -showcerts -connect ejemplo.com:443 </dev/null 2>/dev/null \
| openssl x509 -outform PEM > server.pem
```

Para un broker MQTT (puerto 8883):

```bash
openssl s_client -showcerts -connect broker.tu-dominio.com:8883 </dev/null
```

También puede exportarse desde el navegador o panel de hosting.

---

## 10 ¿A qué se hace referencia cuando se habla de llave pública y privada en el contexto de TLS?

* **Llave pública:** contenida en el certificado; se usa para verificar firmas o cifrar.
* **Llave privada:** mantenida en secreto por el propietario; se usa para firmar o descifrar.
  El conjunto permite autenticación y cifrado seguros.

---

## 11 ¿Qué pasará con el código cuando los certificados expiren?

* La validación TLS fallará con error tipo **CERT_HAS_EXPIRED**.
* Si el firmware ancla el **certificado leaf**, debe recompilarse con uno nuevo.
* Si solo ancla la **CA**, seguirá funcionando si el servidor renueva su certificado con la misma CA.
* Con un **bundle**, solo se requiere actualización ocasional.

---

## 12 ¿Qué teoría matemática es el fundamento de la criptografía moderna? ¿Cuáles son las posibles implicaciones de la computación cuántica para los métodos de criptografía actuales?

La base son ramas de la **teoría de números**, **álgebra** y **complejidad computacional**:

* RSA → factorización de números primos grandes.
* ECC → logaritmo discreto sobre curvas elípticas.
* AES → transformaciones lineales en campos finitos.

### Implicaciones de la computación cuántica

* **Shor:** rompería RSA y ECC.
* **Grover:** reduce seguridad de cifrados simétricos.
   Se impulsa la **criptografía post-cuántica** (algoritmos como Kyber o Dilithium).

---

## 13 Prueba de código — MQTT con TLS

### Paso 1: Conectar a puerto seguro sin validar certificados

```cpp
WiFiClientSecure net;
PubSubClient mqtt(net);

net.setInsecure(); // solo pruebas

mqtt.setServer("broker.tu-dominio.com", 8883);
if (mqtt.connect("esp32-tls-test")) {
  Serial.println("MQTT conectado (inseguro)");
} else {
  Serial.printf("Error: %d\n", mqtt.state());
}
```

**Resultado esperado:** conexión exitosa, pero **sin seguridad real**.

---

### Paso 2: Habilitar validación sin cargar certificados

```cpp
// Sin setInsecure() y sin setCACert()
mqtt.setServer("broker.tu-dominio.com", 8883);
if (mqtt.connect("esp32-tls-validate")) {
  Serial.println("¡Inesperado!");
} else {
  Serial.println("Falla esperada: no hay CA cargada");
}
```

**Resultado esperado:** el handshake TLS falla.

---

### Paso 3: Cargar certificado raíz y validar correctamente

```cpp
const char root_ca[] PROGMEM = R"PEM(
-----BEGIN CERTIFICATE-----
MIIF... (CA completa) ...Q==
-----END CERTIFICATE-----
)PEM";

configTime(0, 0, "pool.ntp.org", "time.nist.gov");
net.setCACert(root_ca);
mqtt.setServer("broker.tu-dominio.com", 8883);

if (mqtt.connect("esp32-tls-ok")) {
  Serial.println("MQTT TLS conectado correctamente");
}
```

**Resultado esperado:** conexión establecida y segura 

---

## Evidencias sugeridas

1. Captura de conexión exitosa sin validación (`setInsecure()`).
2. Captura de fallo sin CA (`verify failed`).
3. Captura de conexión segura con CA cargada.
4. Publicación exitosa a un tópico MQTT.
5. Archivo PEM usado (recortado).

---

## Notas prácticas (ESP32)

* Sincroniza hora antes de conectar:

  ```cpp
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  ```
* Guarda los certificados con `PROGMEM` para ahorrar RAM.
* Si el broker cambia de certificado, actualiza el firmware o el CA.
* No usar `setInsecure()` en producción.
* Usa `setCACertBundle()` si conectas a varios dominios.

---

## Referencias rápidas

| Elemento            | Descripción                            |
| ------------------- | -------------------------------------- |
| Puerto seguro MQTT  | `8883`                                 |
| Validación ON       | `setCACert()` o `setCACertBundle()`    |
| Autenticación mutua | `setCertificate()` + `setPrivateKey()` |
| Bundle (ESP-IDF)    | `esp_crt_bundle_attach`                |
| Fuente de hora      | `NTP (configTime)`                     |


---

## Fuentes y documentación

* [ESP32 Arduino — WiFiClientSecure](https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFiClientSecure/src/WiFiClientSecure.h)
* [ESP-IDF — TLS y bundles](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/protocols/esp_crt_bundle.html)
* [Let’s Encrypt CA](https://letsencrypt.org/)
* [OpenSSL s_client manual](https://www.openssl.org/docs/man1.1.1/man1/openssl-s_client.html)

```
```
