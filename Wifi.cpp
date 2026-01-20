#include "Wifi.h"

// --- CONFIGURATION ---
// Le poisson se connecte au WiFi du PC : SSID + mot de passe [cite: 2]
char ssid[] = "robot_poisson";
char pass[] = "12345678";

int status = WL_IDLE_STATUS;
WiFiServer server(80);

// Prototypes privés
void envoiePageWeb(WiFiClient &client);
void envoieDonneesJSON(WiFiClient &client, Controller &ctrl, Capteurs &caps);
void traiterCommande(String req, Controller &ctrl);

// ============================================================
//   INITIALISATION WIFI (MODE STATION)
// ============================================================
void setupWifi() {
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("[Wifi] Module absent!"); [cite: 2]
    while (true) delay(1000);
  }

  Serial.print("[Wifi] Tentative de connexion a : "); [cite: 2]
  Serial.println(ssid); [cite: 2]

  WiFi.begin(ssid, pass); [cite: 2]

  int tentatives = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("."); [cite: 2]
    tentatives++;
    
    if (tentatives > 20) {
        Serial.println("\n[Wifi] Connexion longue... Verifiez que le PC est en 2.4GHz !"); [cite: 2]
        tentatives = 0;
    }
  }

  Serial.println("");
  Serial.println("[Wifi] CONNECTE au PC !"); [cite: 2]
  server.begin(); [cite: 2]
  printWifiStatus(); [cite: 2]
}

void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(ssid); [cite: 2]

  IPAddress ip = WiFi.localIP(); [cite: 2]
  Serial.print("IP Address: ");
  Serial.println(ip); [cite: 2]
  Serial.print("GO TO: http://");
  Serial.println(ip); [cite: 2]
}

// ============================================================
//   BOUCLE PRINCIPALE DU WIFI
// ============================================================
void gestionServeurWeb(Controller &ctrl, Capteurs &caps) {
  WiFiClient client = server.available(); [cite: 2]
  
  if (client) {
    Serial.println("[Wifi] Client connecte"); [cite: 2]
    String req = "";
    
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        req += c;
        if (c == '\n') break; 
      }
    }

    if (req.indexOf("GET /data") >= 0) {
      envoieDonneesJSON(client, ctrl, caps); [cite: 2]
    } 
    else if (req.indexOf("GET /cmd") >= 0) {
      traiterCommande(req, ctrl); [cite: 2]
      client.println("HTTP/1.1 200 OK\r\nConnection: close\r\n\r\nOK"); [cite: 2]
    }
    else {
      envoiePageWeb(client); [cite: 2]
    }

    while (client.available()) client.read(); 
    client.stop();
    Serial.println("[Wifi] Client deconnecte"); [cite: 2]
  }
}

void traiterCommande(String req, Controller &ctrl) {
  int idx = req.indexOf("key="); [cite: 2]
  if (idx == -1) return;
  
  char key = req.charAt(idx + 4); 
  key = toupper(key); [cite: 2]
  ctrl.onKey(key); [cite: 2]
}

// ============================================================
//   REPONSE JSON /data (CORRIGÉE)
// ============================================================
void envoieDonneesJSON(WiFiClient &client, Controller &ctrl, Capteurs &caps) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();

  const IMUData& imu   = caps.getIMUData(); [cite: 2]
  const PowerData& pwr = caps.getPowerData(); [cite: 2]
  const DepthData& depth = caps.getDepthData(); [cite: 2]
  const LeakData& leak = caps.getLeakData(); [cite: 1, 2]

  client.print("{");
  client.print("\"yaw\":"); client.print(imu.yaw);   client.print(",");
  client.print("\"pit\":"); client.print(imu.pitch); client.print(",");
  client.print("\"rol\":"); client.print(imu.roll);  client.print(",");
  client.print("\"ax\":");  client.print(imu.ax);    client.print(",");
  client.print("\"ay\":");  client.print(imu.ay);    client.print(",");
  client.print("\"az\":");  client.print(imu.az);    client.print(",");
  client.print("\"gx\":");  client.print(imu.gx);    client.print(",");
  client.print("\"gy\":");  client.print(imu.gy);    client.print(",");
  client.print("\"gz\":");  client.print(imu.gz);    client.print(",");

  client.print("\"v\":");   client.print(pwr.busVoltage_V);      client.print(",");
  client.print("\"p\":");   client.print(depth.depth_m);         client.print(","); 

  // CHANGEMENTS ICI :
  // humidity_percent n'existe pas dans Capteurs.cpp, on met 0 par défaut 
  client.print("\"hum\":"); client.print(0); client.print(","); 
  // leakDetected est remplacé par leakNow qui existe dans la structure LeakData 
  client.print("\"leak\":"); client.print(leak.leakNow ? "true" : "false"); client.print(",");

  client.print("\"auto\":"); 
  client.print(ctrl.mode() == ControlMode::AUTONOMOUS ? "true" : "false"); [cite: 2]
  client.print("}");
}

// ============================================================
//   INTERFACE HTML
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
".label { font-size: 0.8em; color: #94a3b8; }"
".controls { background: #1e293b; padding: 20px; border-radius: 15px; margin-top: 10px; }"
".key-row { display: flex; justify-content: center; gap: 10px; margin: 10px 0; }"
".key { background: #334155; padding: 15px 25px; border-radius: 8px; font-weight: bold;"
"       border: 2px solid #475569; cursor:pointer; user-select: none; color:#e2e8f0;"
"       transition: background 0.1s, transform 0.05s; }"
".key:active { background: #22c55e; color: #000; border-color: #ffffff; transform: translateY(2px); }"
".key.active { background: #22c55e; color: #000; border-color: #ffffff; transform: translateY(2px); }"
"</style></head><body>"
"<div class='container'>"
"<h1>COMMANDER CENTER</h1>"
"<div class='dashboard'>"
  "<div class='card'><div class='label'>MODE</div><div class='val' id='mode'>MANUEL</div></div>"
  "<div class='card'><div class='label'>BATTERIE</div><div class='val'><span id='vbat'>--</span> V</div></div>"
  "<div class='card'><div class='label'>PROFONDEUR</div><div class='val'><span id='prof'>--</span> m</div></div>"
  "<div class='card'><div class='label'>CAP (YAW)</div><div class='val'><span id='yaw'>--</span> deg</div></div>"
"</div>"
"<div class='controls'>"
  "<h3>PILOTAGE (Clavier ZQSD + A)</h3>"
  "<div class='key-row'><div class='key' id='kZ' onclick=\"sendCmd('Z')\">Z</div></div>"
  "<div class='key-row'>"
"    <div class='key' id='kQ' onclick=\"sendCmd('Q')\">Q</div>"
"    <div class='key' id='kS' onclick=\"sendCmd('S')\">S</div>"
"    <div class='key' id='kD' onclick=\"sendCmd('D')\">D</div>"
  "</div>"
  "<div class='key-row'><div class='key' id='kA' onclick=\"sendCmd('A')\">A (AUTO)</div></div>"
"</div>"
"</div>"
"<script>"
"function sendCmd(k){fetch('/cmd?key='+k);}"
"window.addEventListener('keydown',function(e){"
"  var k=e.key.toUpperCase();"
"  if(['Z','Q','S','D','A'].includes(k)){"
"    if(e.repeat)return;"
"    document.getElementById('k'+k).classList.add('active');"
"    sendCmd(k);"
"  }"
"});"
"window.addEventListener('keyup',function(e){"
"  var k=e.key.toUpperCase();"
"  if(['Z','Q','S','D','A'].includes(k)){"
"    document.getElementById('k'+k).classList.remove('active');"
"    if(['Z','Q','S','D'].includes(k)) sendCmd('S');"
"  }"
"});"
"setInterval(function(){"
"  fetch('/data').then(r=>r.json()).then(d=>{"
"    document.getElementById('vbat').innerText=d.v.toFixed(2);"
"    document.getElementById('prof').innerText=d.p.toFixed(2);"
"    document.getElementById('yaw').innerText=d.yaw.toFixed(0);"
"    document.getElementById('mode').innerText=d.auto?'AUTONOME':'MANUEL';"
"  });"
"},200);"
"</script></body></html>"
  ));
}