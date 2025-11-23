#pragma once
#include <pgmspace.h>

// Página principal con controles, estado y radar en p5.js
static const char INDEX_HTML[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="UTF-8">
<title>Carro IoT ESP32</title>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<style>
body { font-family: Arial, sans-serif; background:#111; color:#eee; text-align:center; }
button { margin:6px; padding:10px 18px; font-size:16px; }
#radarContainer { margin-top:15px; display:flex; justify-content:center; }
.status { margin-top:10px; font-size:14px; }
</style>

<!-- p5.js solo para gráficos -->
<script src="https://cdn.jsdelivr.net/npm/p5@1.9.0/lib/p5.min.js"></script>
</head>
<body>
<h2>Carro IoT ESP32</h2>
<div>
  <button onclick="moveDir('forward')">Adelante</button><br>
  <button onclick="moveDir('left')">Izquierda</button>
  <button onclick="moveDir('stop')">Stop</button>
  <button onclick="moveDir('right')">Derecha</button><br>
  <button onclick="moveDir('backward')">Atrás</button>
</div>
<div class="status">
  <span id="statusText">Radar local (REST)</span><br>
  Distancia: <span id="distText">--</span> cm
</div>

<div id="radarContainer"></div>

<script>
// ======================= REST para movimiento =======================
const apiBase = window.location.origin;

function moveDir(dir) {
  fetch(apiBase + '/api/v1/move?dir=' + dir + '&speed=200', {
    method: 'POST'
  }).catch(err => console.error("[REST] Error:", err));
}

// ======================= Datos para radar =======================
let radarPoints = [];   // {angle, dist}
const MAX_DIST_CM = 200;   // radio máximo 2 m
const OBSTACLE_CM = 30;    // por debajo => obstáculo en rojo

function updateRadar(points) {
  radarPoints = points || [];
  if (radarPoints.length > 0) {
    const last = radarPoints[radarPoints.length - 1];
    if (last.distance_cm != null) {
      document.getElementById('distText').textContent =
        last.distance_cm.toFixed(1);
    }
  }
}

// Polling periódico al ESP32
async function fetchRadar() {
  try {
    const res = await fetch(apiBase + "/api/v1/radar");
    if (!res.ok) return;
    const data = await res.json(); // array [{angle_deg, distance_cm}, ...]
    updateRadar(data);
  } catch (e) {
    console.error("[RADAR] Error al obtener datos:", e);
  }
}
setInterval(fetchRadar, 400);
fetchRadar();

// ======================= Sketch de p5.js =======================
new p5((sketch) => {
  let cx, cy, maxRadius;

  sketch.setup = () => {
    const canvas = sketch.createCanvas(400, 220); // ancho 400, alto 220
    canvas.parent('radarContainer');
    cx = sketch.width / 2;
    cy = sketch.height - 10;        // centro en la parte de abajo
    maxRadius = Math.min(cx, cy) - 10;
  };

  sketch.draw = () => {
    sketch.background(0);

    // semicírculos (180 grados frontales)
    sketch.noFill();
    sketch.stroke(0, 255, 0);
    sketch.arc(cx, cy, 2 * maxRadius, 2 * maxRadius, Math.PI, 2 * Math.PI);
    sketch.arc(cx, cy, maxRadius, maxRadius, Math.PI, 2 * Math.PI);

    // dibujar puntos
    radarPoints.forEach(p => {
      if (p.distance_cm == null) return;

      const dist = Math.min(p.distance_cm, MAX_DIST_CM);
      const r = dist * (maxRadius / MAX_DIST_CM);

      // El servo escanea aprox de 20° (izq) a 160° (der).
      // Normalizamos a [0..1] y lo mapeamos al semicírculo [PI..0].
      const norm = (p.angle_deg - 20) / 140.0;  // 0 = izquierda, 1 = derecha
      const theta = Math.PI * (1.0 - norm);     // izquierda PI, derecha 0

      const x = cx + r * Math.cos(theta);
      const y = cy - r * Math.sin(theta);

      // Verde si lejos, rojo si cerca
      if (dist <= OBSTACLE_CM) {
        sketch.fill(255, 0, 0);
      } else {
        sketch.fill(0, 255, 0);
      }
      sketch.noStroke();
      sketch.ellipse(x, y, 6, 6);
    });

    // pequeña línea de frente del carro
    sketch.stroke(0, 180, 0);
    sketch.line(cx, cy, cx, cy - maxRadius);
  };
});
</script>
</body>
</html>
)HTML";
