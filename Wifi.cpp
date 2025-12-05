#include "Wifi.h"

// --- CONFIGURATION ---
char ssid[] = "robot_poisson";
char pass[] = "12345678";

int status = WL_IDLE_STATUS;
WiFiServer server(80);

// Prototypes privés
void envoiePageWeb(WiFiClient &client);
void envoieDonneesJSON(WiFiClient &client, Controller &ctrl, Capteurs &caps);
void traiterCommande(String req, Controller &ctrl);

void setupWifi() {
  // Vérif module
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("[Wifi] Module absent!");
    while (true);
  }

  // Connexion
  while (status != WL_CONNECTED) {
    Serial.print("[Wifi] Connexion a: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(5000);
  }

  server.begin();
  printWifiStatus();
}

void printWifiStatus() {
  Serial.print("SSID: "); Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: "); Serial.println(ip);
  Serial.print("GO TO: http://"); Serial.println(ip);
}

// ============================================================
//   BOUCLE PRINCIPALE DU WIFI
// ============================================================
void gestionServeurWeb(Controller &ctrl, Capteurs &caps) {
  WiFiClient client = server.available();
  
  if (client) {
    String req = "";
    boolean currentLineIsBlank = true;
    
    // Lecture de la requête (limité à la première ligne pour la vitesse)
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        req += c;
        if (c == '\n') break; 
      }
    }

    // --- AIGUILLAGE ---
    
    // 1. Demande de données (Mise à jour interface JS)
    if (req.indexOf("GET /data") >= 0) {
      envoieDonneesJSON(client, ctrl, caps);
    } 
    // 2. Commande (Touche Z,Q,S,D ou A appuyée)
    else if (req.indexOf("GET /cmd") >= 0) {
      traiterCommande(req, ctrl);
      // Réponse vide rapide pour ne pas bloquer le navigateur
      client.println("HTTP/1.1 200 OK\r\nConnection: close\r\n\r\nOK");
    }
    // 3. Sinon, on envoie la page HTML complète
    else {
      envoiePageWeb(client);
    }

    // Nettoyage buffer
    while(client.available()) client.read(); 
    client.stop();
  }
}

// ============================================================
//   TRAITEMENT
// ============================================================

void traiterCommande(String req, Controller &ctrl) {
  int idx = req.indexOf("key=");
  if (idx == -1) return;
  
  char key = req.charAt(idx + 4); // Récupère le caractère après "key="
  
  // MAGIE : On passe la touche directement au Controller !
  // Le Wifi ne sait pas ce que fait 'Z', c'est le Controller qui gère.
  ctrl.onKey(key); 
}

void envoieDonneesJSON(WiFiClient &client, Controller &ctrl, Capteurs &caps) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();

  // Récupération des données via les méthodes publiques
  const IMUData& imu = caps.getIMUData();
  const PowerData& pwr = caps.getPowerData();
  const DepthData& depth = caps.getDepthData();
  const LeakData& leak = caps.getLeakData();

  // Construction JSON
  client.print("{");
  
  // IMU
  client.print("\"yaw\":"); client.print(imu.yaw); client.print(",");
  client.print("\"pit\":"); client.print(imu.pitch); client.print(",");
  client.print("\"rol\":"); client.print(imu.roll); client.print(",");
  client.print("\"ax\":"); client.print(imu.ax); client.print(",");
  client.print("\"ay\":"); client.print(imu.ay); client.print(",");
  client.print("\"az\":"); client.print(imu.az); client.print(",");
  client.print("\"gx\":"); client.print(imu.gx); client.print(",");
  client.print("\"gy\":"); client.print(imu.gy); client.print(",");
  client.print("\"gz\":"); client.print(imu.gz); client.print(",");

  // Power & Environment
  client.print("\"v\":"); client.print(pwr.busVoltage_V); client.print(",");
  client.print("\"p\":"); client.print(depth.depth_m); client.print(","); // Profondeur
  client.print("\"hum\":"); client.print(leak.humidity_percent); client.print(",");
  client.print("\"leak\":"); client.print(leak.leakDetected ? "true" : "false"); client.print(",");

  // État du controleur
  client.print("\"auto\":"); 
  client.print(ctrl.mode() == ControlMode::AUTONOMOUS ? "true" : "false");
  
  client.print("}");
}

// ============================================================
//   INTERFACE HTML (Stockée en mémoire programme F())
// ============================================================
void envoiePageWeb(WiFiClient &client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  
  client.print(F(
    "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<title>Robot Poisson MKR</title>"
    "<style>"
    "body { background: #0f172a; color: #e2e8f0; font-family: sans-serif; text-align: center; margin: 0; padding: 20px; }"
    ".container { max-width: 800px; margin: auto; }"
    "h1 { color: #38bdf8; }"
    ".dashboard { display: grid; grid-template-columns: repeat(auto-fit, minmax(150px, 1fr)); gap: 15px; margin-bottom: 20px; }"
    ".card { background: #1e293b; padding: 15px; border-radius: 10px; box-shadow: 0 4px 6px rgba(0,0,0,0.3); }"
    ".val { font-size: 1.2em; font-weight: bold; color: #34d399; }"
    ".warn { color: #ef4444; }"
    ".label { font-size: 0.8em; color: #94a3b8; }"
    ".controls { background: #1e293b; padding: 20px; border-radius: 15px; }"
    ".key-row { display: flex; justify-content: center; gap: 10px; margin: 5px; }"
    ".key { background: #334155; padding: 15px 25px; border-radius: 8px; font-weight: bold; border: 2px solid #475569; cursor:pointer; user-select: none; }"
    ".key.active { background: #38bdf8; color: #000; border-color: #fff; }"
    ".mode-btn { background: #ef4444; color: white; padding: 10px 20px; border-radius: 5px; border: none; font-size: 1.2em; margin-top: 20px; cursor: pointer; }"
    ".mode-btn.auto { background: #22c55e; }"
    "</style></head><body>"
    "<div class='container'>"
    "<h1>COMMANDER CENTER</h1>"
    
    "<div class='dashboard'>"
      "<div class='card'><div class='label'>MODE</div><div class='val' id='mode'>MANUEL</div></div>"
      "<div class='card'><div class='label'>BATTERIE</div><div class='val'><span id='vbat'>--</span> V</div></div>"
      "<div class='card'><div class='label'>PROFONDEUR</div><div class='val'><span id='prof'>--</span> m</div></div>"
      "<div class='card'><div class='label'>CAP (YAW)</div><div class='val'><span id='yaw'>--</span> deg</div></div>"
      "<div class='card'><div class='label'>FUITE</div><div class='val' id='leak'>NON</div></div>"
    "</div>"

    "<div class='dashboard' style='font-size:0.8em'>"
       "<div class='card'><div class='label'>ACCEL (X/Y/Z)</div><div id='acc'>--</div></div>"
       "<div class='card'><div class='label'>GYRO (X/Y/Z)</div><div id='gyr'>--</div></div>"
       "<div class='card'><div class='label'>ATTITUDE (P/R)</div><div id='att'>--</div></div>"
    "</div>"

    "<div class='controls'>"
      "<h3>PILOTAGE (Clavier ZQSD)</h3>"
      "<div class='key-row'><div class='key' id='kZ' onmousedown='d(87)' onmouseup='u(87)'>Z</div></div>"
      "<div class='key-row'>"
        "<div class='key' id='kQ' onmousedown='d(81)' onmouseup='u(81)'>Q</div>"
        "<div class='key' id='kS' onmousedown='d(83)' onmouseup='u(83)'>S</div>"
        "<div class='key' id='kD' onmousedown='d(68)' onmouseup='u(68)'>D</div>"
      "</div>"
      "<button class='mode-btn' id='btnMode' onclick='sendCmd(\"A\")'>ACTIVER MODE AUTONOME (A)</button>"
    "</div>"
    "</div>"

    "<script>"
    "function sendCmd(k) { fetch('/cmd?key='+k); }"
    
    // Gestion Clavier et Souris unifiée
    "function d(c) { let k=String.fromCharCode(c); document.getElementById('k'+k).classList.add('active'); sendCmd(k); }"
    "function u(c) { let k=String.fromCharCode(c); document.getElementById('k'+k).classList.remove('active'); sendCmd('S'); }" // S = STOP quand on relache

    "document.addEventListener('keydown', function(e) {"
      "if(e.repeat) return;"
      "let c = e.keyCode;"
      "if(c==65) sendCmd('A');" // Touche A
      "if([87,81,83,68].includes(c)) d(c);" // ZQSD
    "});"
    
    "document.addEventListener('keyup', function(e) {"
      "let c = e.keyCode;"
      "if([87,81,83,68].includes(c)) u(c);"
    "});"

    // Boucle de mise à jour (5Hz)
    "setInterval(function() {"
      "fetch('/data').then(r => r.json()).then(d => {"
        "document.getElementById('vbat').innerText = d.v.toFixed(2);"
        "document.getElementById('prof').innerText = d.p.toFixed(2);"
        "document.getElementById('yaw').innerText = d.yaw.toFixed(0);"
        "document.getElementById('acc').innerText = d.ax.toFixed(1) + '/' + d.ay.toFixed(1) + '/' + d.az.toFixed(1);"
        "document.getElementById('gyr').innerText = d.gx.toFixed(1) + '/' + d.gy.toFixed(1) + '/' + d.gz.toFixed(1);"
        "document.getElementById('att').innerText = d.pit.toFixed(0) + '/' + d.rol.toFixed(0);"
        
        "let leakEl = document.getElementById('leak');"
        "leakEl.innerText = d.leak ? 'ALERTE !' : 'OK';"
        "leakEl.className = d.leak ? 'val warn' : 'val';"
        
        "document.getElementById('mode').innerText = d.auto ? 'AUTONOME' : 'MANUEL';"
        "let btn = document.getElementById('btnMode');"
        "if(d.auto) { btn.classList.add('auto'); btn.innerText = 'DESACTIVER AUTONOME (A)'; }"
        "else { btn.classList.remove('auto'); btn.innerText = 'ACTIVER MODE AUTONOME (A)'; }"
      "});"
    "}, 200);"
    "</script></body></html>"
  ));
}