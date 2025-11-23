#pragma once
#include <pgmspace.h>

// UI con radar preciso (bins por ángulo + persistencia), motion trail,
// MQTT por WSS, tema claro/oscuro y control por teclado/D-pad.
static const char INDEX_HTML[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="UTF-8" />
<meta name="viewport" content="width=device-width, initial-scale=1" />
<title>Carro IoT ESP32</title>

<script src="https://cdn.jsdelivr.net/npm/p5@1.9.0/lib/p5.min.js"></script>
<script src="https://cdn.jsdelivr.net/npm/mqtt@5.8.0/dist/mqtt.min.js"></script>

<style>
  :root{ --bg:#0b1220; --card:#0f172a; --txt:#e5e7eb; --muted:#9fb0c7; --edge:#1f2a40; --accent:#22d3ee; --ok:#22c55e; --danger:#ef4444; --btn:#111827; }
  .light{ --bg:#f6f7fb; --card:#ffffff; --txt:#0f172a; --muted:#475569; --edge:#e6e9f0; --accent:#0891b2; --ok:#16a34a; --danger:#dc2626; --btn:#f3f4f6; }
  *{box-sizing:border-box} html,body{height:100%}
  body{margin:0;background:var(--bg);color:var(--txt);font-family:system-ui,-apple-system,Segoe UI,Roboto,Ubuntu;display:flex;justify-content:center}
  .wrap{width:100%;max-width:980px;padding:16px;display:flex;flex-direction:column;gap:16px}
  .card{background:var(--card);border:1px solid var(--edge);border-radius:18px;box-shadow:0 12px 30px rgba(0,0,0,.15)}
  .header{display:flex;align-items:center;justify-content:space-between;padding:14px 16px;border-bottom:1px solid var(--edge)}
  .h1{font-weight:800;letter-spacing:.2px}
  .pill{padding:6px 10px;border-radius:999px;border:1px solid var(--edge);color:var(--muted)}
  .accent{color:var(--accent)} .ok{color:var(--ok)} .danger{color:var(--danger)}
  .toggle{padding:8px 10px;border-radius:12px;background:var(--btn);border:1px solid var(--edge);cursor:pointer;font-weight:700}
  .radarBox{padding:10px 14px} #radarContainer{display:flex;justify-content:center}
  .controls{padding:16px;display:grid;gap:16px;grid-template-columns:1fr 280px}
  @media (max-width:860px){.controls{grid-template-columns:1fr}}
  .statusBox{display:flex;gap:12px;align-items:center;flex-wrap:wrap}
  .statusItem{padding:10px 12px;border:1px solid var(--edge);border-radius:12px}
  .big{font-weight:700}
  .kbd{font-family:ui-monospace,SFMono-Regular,Menlo,Monaco;font-size:.85rem;padding:.1rem .45rem;border-radius:6px;border:1px solid var(--edge)}
  .dpad{display:grid;gap:10px;grid-template-columns:repeat(3,95px);grid-template-rows:repeat(3,95px);justify-content:center}
  .btn{background:var(--btn);color:var(--txt);border:1px solid var(--edge);border-radius:16px;font-weight:800;cursor:pointer;display:flex;align-items:center;justify-content:center;transition:.06s transform,.15s box-shadow}
  .btn:hover{box-shadow:0 6px 18px rgba(0,0,0,.18)} .btn:active{transform:scale(.98)}
  .up{grid-column:2;grid-row:1}.left{grid-column:1;grid-row:2}.center{grid-column:2;grid-row:2}.right{grid-column:3;grid-row:2}.down{grid-column:2;grid-row:3}
  .btn.stop{background:linear-gradient(180deg,#7f1d1d,#991b1b);border-color:#b91c1c}
  .carSvg{width:46px;height:46px}
  .hint{color:var(--muted);font-size:.9rem;text-align:center}
</style>
</head>
<body>
  <div class="wrap">
    <div class="card">
      <div class="header">
        <div class="h1">Carro IoT ESP32</div>
        <div style="display:flex;gap:10px;align-items:center">
          <span class="pill">Ángulo: <span id="angText" class="big accent">--</span>°</span>
          <span class="pill">Distancia: <span id="distText" class="big accent">--</span> cm</span>
          <span class="pill" id="mqttPill">MQTT: <span class="danger">DESC</span></span>
          <button id="themeBtn" class="toggle">🌗 Tema</button>
        </div>
      </div>

      <div class="radarBox"><div id="radarContainer"></div></div>

      <div class="controls">
        <div class="statusBox">
          <div class="statusItem">Teclado: <span class="kbd">↑</span> <span class="kbd">↓</span> <span class="kbd">←</span> <span class="kbd">→</span> · <span class="kbd">Espacio</span>=Stop</div>
          <div class="statusItem">REST base: <span id="baseUrl" class="accent"></span></div>
          <div class="statusItem">Velocidad fija: <span class="big">200</span></div>
        </div>

        <div class="dpad">
          <button class="btn up" onclick="sendMove('forward')">↑</button>
          <button class="btn left" onclick="sendMove('left')">←</button>
          <button class="btn stop center" onclick="sendMove('stop')">
            <svg class="carSvg" viewBox="0 0 64 64" aria-hidden="true">
              <rect x="12" y="24" width="40" height="16" rx="3" fill="#111" stroke="#eee" stroke-width="2"></rect>
              <rect x="18" y="18" width="28" height="10" rx="3" fill="#111" stroke="#eee" stroke-width="2"></rect>
              <circle cx="20" cy="42" r="6" fill="#222" stroke="#eee" stroke-width="2"></circle>
              <circle cx="44" cy="42" r="6" fill="#222" stroke="#eee" stroke-width="2"></circle>
              <rect x="28" y="26" width="8" height="12" rx="2" fill="#ef4444" stroke="#fff" stroke-width="1.5"></rect>
            </svg>
          </button>
          <button class="btn right" onclick="sendMove('right')">→</button>
          <button class="btn down" onclick="sendMove('backward')">↓</button>
        </div>
      </div>
    </div>

    <div class="hint">Mantén presionadas las flechas para movimiento continuo; al soltar se envía STOP.</div>
  </div>

<script>
/* -------- Tema -------- */
(function(){
  const root=document.documentElement;
  const saved=localStorage.getItem('theme')||'dark';
  if(saved==='light') root.classList.add('light');
  document.getElementById('themeBtn').addEventListener('click',()=>{
    root.classList.toggle('light');
    localStorage.setItem('theme', root.classList.contains('light')?'light':'dark');
  });
})();

/* -------- Movimiento (REST) -------- */
const apiBase = window.location.origin;
document.getElementById('baseUrl').textContent = apiBase;
const FIXED_SPEED=200;
let keyState={up:false,down:false,left:false,right:false};
function sendMove(dir){ fetch(`${apiBase}/api/v1/move?dir=${dir}&speed=${FIXED_SPEED}`,{method:'POST'}).catch(()=>{}); }
function handleKeys(e,down){
  if(e.repeat) return;
  switch(e.key){
    case 'ArrowUp':    keyState.up=down; break;
    case 'ArrowDown':  keyState.down=down; break;
    case 'ArrowLeft':  keyState.left=down; break;
    case 'ArrowRight': keyState.right=down; break;
    case ' ': if(down) sendMove('stop'); return;
    default: return;
  }
  if(keyState.up && !keyState.down && !keyState.left && !keyState.right)      sendMove('forward');
  else if(keyState.down && !keyState.up && !keyState.left && !keyState.right)  sendMove('backward');
  else if(keyState.left && !keyState.right && !keyState.up && !keyState.down)  sendMove('left');
  else if(keyState.right && !keyState.left && !keyState.up && !keyState.down)  sendMove('right');
  else if(!keyState.up && !keyState.down && !keyState.left && !keyState.right) sendMove('stop');
}
window.addEventListener('keydown',e=>handleKeys(e,true));
window.addEventListener('keyup',e=>handleKeys(e,false));

/* -------- MQTT (WSS) -------- */
const BROKER_URL = 'wss://test.mosquitto.org:8081/mqtt';
const TOPIC_DIST = 'carro/distancia';
const TOPIC_MAP  = 'carro/mapa';

// Si tu servo barre 20°..160° cámbialo aquí si difiere:
const ANGLE_MIN_UI = 20;
const ANGLE_MAX_UI = 160;

let client;
function connectMqtt(){
  const cid='web-'+Math.random().toString(16).slice(2,8);
  client=mqtt.connect(BROKER_URL,{clean:true,clientId:cid,connectTimeout:6000,reconnectPeriod:2000,protocolVersion:4});
  client.on('connect',()=>{ document.getElementById('mqttPill').innerHTML='MQTT: <span class="ok">OK</span>'; client.subscribe([TOPIC_DIST,TOPIC_MAP]); });
  client.on('close',()=>{ document.getElementById('mqttPill').innerHTML='MQTT: <span class="danger">DESC</span>'; });
  client.on('message',(topic,payload)=>{
    let data; try{ data=JSON.parse(new TextDecoder().decode(payload)); }catch(_){ return; }
    const now=performance.now();
    if(topic===TOPIC_DIST){
      if(data.distance_cm!=null){ lastDist=Number(data.distance_cm); document.getElementById('distText').textContent=lastDist.toFixed(1); }
    }else if(topic===TOPIC_MAP){
      if(typeof data.angle_deg==='number'){
        const a=Number(data.angle_deg);
        lastAngle=a; document.getElementById('angText').textContent=Math.round(a);
        // Guardar en bin
        const ai = Math.max(ANGLE_MIN_UI, Math.min(ANGLE_MAX_UI, Math.round(a)));
        angleBins[ai] = { t: now, d: (data.distance_cm==null? null : Number(data.distance_cm)) };
        // Motion trail
        trail.push({t:now, a:a, d:(data.distance_cm==null? null : Number(data.distance_cm))});
        if(trail.length>600) trail.shift();
        // Última distancia texto
        if(data.distance_cm!=null){ lastDist=Number(data.distance_cm); document.getElementById('distText').textContent=lastDist.toFixed(1); }
      }
    }
  });
}
connectMqtt();

/* -------- Radar preciso (bins + persistencia + trail) -------- */
const MAX_DIST_CM=200;
const OBSTACLE_CM=30;

// un bin por grado (20..160)
const angleBins = {};
let trail = [];            // últimos impactos (para estela)
let lastAngle=90, lastDist=null;

new p5((sk)=>{
  let cx,cy,maxR;

  sk.setup=()=>{
    const c=sk.createCanvas(Math.min(940, sk.windowWidth-40), 360);
    c.parent('radarContainer');
    cx=sk.width/2; cy=sk.height-10; maxR=Math.min(cx,cy)-10;
  };
  sk.windowResized=()=>{
    const w=Math.min(940, sk.windowWidth-40);
    sk.resizeCanvas(w,360); cx=sk.width/2; cy=sk.height-10; maxR=Math.min(cx,cy)-10;
  };

  const gridCol = ()=> document.documentElement.classList.contains('light') ? '#0ea5e9' : '#22d3ee';
  const ringCol = ()=> document.documentElement.classList.contains('light') ? '#16a34a' : '#22c55e';

  function drawGrid(){
    const light=document.documentElement.classList.contains('light');
    sk.background(light?255:0);
    // anillos
    sk.noFill(); sk.stroke(ringCol()); sk.strokeWeight(2);
    sk.arc(cx,cy,2*maxR,2*maxR,Math.PI,2*Math.PI);
    sk.arc(cx,cy,1.6*maxR,1.6*maxR,Math.PI,2*Math.PI);
    sk.arc(cx,cy,1.2*maxR,1.2*maxR,Math.PI,2*Math.PI);
    sk.arc(cx,cy,0.8*maxR,0.8*maxR,Math.PI,2*Math.PI);
    // radiales
    sk.stroke(gridCol()); sk.strokeWeight(1.5);
    for(let a=30;a<=150;a+=30){
      const t=sk.radians(180-a);
      sk.line(cx,cy, cx+maxR*Math.cos(t), cy-maxR*Math.sin(t));
      sk.noStroke(); sk.fill(gridCol()); sk.textSize(12); sk.textAlign(sk.CENTER,sk.CENTER);
      const lx=cx+(maxR+16)*Math.cos(t), ly=cy-(maxR+16)*Math.sin(t);
      sk.text(`${a}°`, lx, ly);
    }
    // central
    sk.stroke(gridCol()); sk.line(cx,cy,cx,cy-maxR);
  }

  function drawBins(){
    const now=performance.now();
    // fondo “persistente” verde con decaimiento (hasta 2.5 s)
    Object.keys(angleBins).forEach(k=>{
      const a=Number(k), item=angleBins[k];
      if(!item) return;
      const age = (now - item.t);
      if(age>2500) return; // caducado
      const alpha = Math.max(0, 1 - age/2500);  // decaimiento
      const dist = (item.d==null) ? MAX_DIST_CM : Math.min(item.d, MAX_DIST_CM);
      const r = dist*(maxR/MAX_DIST_CM);

      const t = sk.radians(180-a);
      const w = sk.radians(1.2); // barra fina mejor alineada
      const near = (item.d!=null && item.d<=OBSTACLE_CM);

      sk.noStroke();
      const col = near
        ? (document.documentElement.classList.contains('light') ? [220,38,38] : [239,68,68])
        : (document.documentElement.classList.contains('light') ? [22,163,74] : [34,197,94]);
      sk.fill(col[0], col[1], col[2], 180*alpha);
      sk.beginShape();
      sk.vertex(cx,cy);
      sk.vertex(cx + r*Math.cos(t-w), cy - r*Math.sin(t-w));
      sk.vertex(cx + r*Math.cos(t+w), cy - r*Math.sin(t+w));
      sk.endShape(sk.CLOSE);
    });
  }

  function drawTrail(){
    // estela verde (sólo impactos recientes del haz, 900 ms)
    const now=performance.now();
    sk.noStroke();
    trail.forEach(p=>{
      const age=now - p.t;
      if(age>900 || p.d==null) return;
      const alpha = 1 - age/900;
      const dist=Math.min(p.d, MAX_DIST_CM);
      const r = dist*(maxR/MAX_DIST_CM);
      const t = sk.radians(180-p.a);
      sk.fill( (document.documentElement.classList.contains('light')?22:34),
               163, (document.documentElement.classList.contains('light')?74:94), 220*alpha );
      sk.ellipse(cx + r*Math.cos(t), cy - r*Math.sin(t), 6, 6);
    });
  }

  function drawSweep(){
    const t=sk.radians(180-lastAngle);
    sk.fill( document.documentElement.classList.contains('light') ? 'rgba(8,145,178,.18)' : 'rgba(34,211,238,.22)' );
    sk.noStroke();
    const r=maxR, w=sk.radians(3.2);
    sk.beginShape();
    sk.vertex(cx,cy);
    sk.vertex(cx + r*Math.cos(t-w), cy - r*Math.sin(t-w));
    sk.vertex(cx + r*Math.cos(t+w), cy - r*Math.sin(t+w));
    sk.endShape(sk.CLOSE);
  }

  sk.draw=()=>{ drawGrid(); drawBins(); drawTrail(); drawSweep(); };
});
</script>
</body>
</html>
)HTML";
