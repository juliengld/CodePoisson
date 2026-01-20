#include "Wifi.h"

// --- CONFIGURATION ---
// Le poisson crée son propre WiFi : SSID + mot de passe
char ssid[] = "robot_poisson";
char pass[] = "12345678";

int status = WL_IDLE_STATUS;
WiFiServer server(80);

// Prototypes privés
void envoiePageWeb(WiFiClient &client);
void envoieDonneesJSON(WiFiClient &client, Controller &ctrl, Capteurs &caps);
void traiterCommande(String req, Controller &ctrl);

// ============================================================
//   INITIALISATION WIFI (POINT D'ACCES)
// ============================================================
// DANS Wifi.cpp

void setupWifi() {
  // Vérif module WiFi
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("[Wifi] Module absent!");
    while (true) delay(1000);
  }

  // ----------------------------------------------------
  // NOUVELLE LOGIQUE : CONNEXION AU PC (MODE STATION)
  // ----------------------------------------------------
  Serial.print("[Wifi] Tentative de connexion a : ");
  Serial.println(ssid);

  // On lance la connexion
  WiFi.begin(ssid, pass);

  // On attend que le statut passe à "CONNECTED"
  // On fait clignoter des points ... dans le moniteur série
  int tentatives = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    tentatives++;
    
    // Si ça prend plus de 10 secondes, petit message d'aide
    if (tentatives > 20) {
        Serial.println("\n[Wifi] Connexion longue... Verifiez que le PC est en 2.4GHz !");
        tentatives = 0;
    }
  }

  Serial.println("");
  Serial.println("[Wifi] CONNECTE au PC !");

  // Démarrage du serveur web
  server.begin();
  
  // Affiche la nouvelle IP (C'est le PC qui l'a donnée)
  printWifiStatus();
}

void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(ssid);

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.print("GO TO: http://");
  Serial.println(ip);
}

// ============================================================
//   BOUCLE PRINCIPALE DU WIFI
// ============================================================
void gestionServeurWeb(Controller &ctrl, Capteurs &caps) {
  WiFiClient client = server.available();
  
  if (client) {
    Serial.println("[Wifi] Client connecte");

    String req = "";
    
    // Lecture de la première ligne de la requête
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        req += c;
        if (c == '\n') break; // on lit juste la ligne GET /... HTTP/1.1
      }
    }

    Serial.print("[Wifi] Requete: ");
    Serial.println(req);

    // --- AIGUILLAGE ---
    if (req.indexOf("GET /data") >= 0) {
      envoieDonneesJSON(client, ctrl, caps);
    } 
    else if (req.indexOf("GET /cmd") >= 0) {
      Serial.println("[Wifi] Requete CMD detectee");
      traiterCommande(req, ctrl);
      client.println("HTTP/1.1 200 OK\r\nConnection: close\r\n\r\nOK");
    }
    else {
      Serial.println("[Wifi] Envoi page HTML");
      envoiePageWeb(client);
    }

    while (client.available()) client.read(); 
    client.stop();
    Serial.println("[Wifi] Client deconnecte");
  }
}


// ============================================================
//   TRAITEMENT COMMANDE
// ============================================================
void traiterCommande(String req, Controller &ctrl) {
  int idx = req.indexOf("key=");
  if (idx == -1) return;
  
  char key = req.charAt(idx + 4); // caractère après "key="
  key = toupper(key);

  Serial.print("[Wifi] Commande reçue: ");
  Serial.println(key);

  ctrl.onKey(key); 
}



// ============================================================
//   REPONSE JSON /data
// ============================================================
void envoieDonneesJSON(WiFiClient &client, Controller &ctrl, Capteurs &caps) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();

  // Récupération des données via les méthodes publiques
  const IMUData& imu   = caps.getIMUData();
  const PowerData& pwr = caps.getPowerData();
  const DepthData& depth = caps.getDepthData();
  const LeakData& leak = caps.getLeakData();

  // Construction JSON
  client.print("{");
  
  // IMU
  client.print("\"yaw\":"); client.print(imu.yaw);   client.print(",");
  client.print("\"pit\":"); client.print(imu.pitch); client.print(",");
  client.print("\"rol\":"); client.print(imu.roll);  client.print(",");
  client.print("\"ax\":");  client.print(imu.ax);    client.print(",");
  client.print("\"ay\":");  client.print(imu.ay);    client.print(",");
  client.print("\"az\":");  client.print(imu.az);    client.print(",");
  client.print("\"gx\":");  client.print(imu.gx);    client.print(",");
  client.print("\"gy\":");  client.print(imu.gy);    client.print(",");
  client.print("\"gz\":");  client.print(imu.gz);    client.print(",");

  // Power & Environment
  client.print("\"v\":");   client.print(pwr.busVoltage_V);      client.print(",");
  client.print("\"p\":");   client.print(depth.depth_m);         client.print(","); // Profondeur
  client.print("\"hum\":"); client.print(leak.humidity_percent); client.print(",");
  client.print("\"leak\":"); client.print(leak.leakDetected ? "true" : "false"); client.print(",");

  // État du controleur
  client.print("\"auto\":"); 
  client.print(ctrl.mode() == ControlMode::AUTONOMOUS ? "true" : "false");
  
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

"<div class='dashboard' style='font-size:0.8em'>"
  "<div class='card'><div class='label'>ACCEL (X/Y/Z)</div><div id='acc'>--</div></div>"
  "<div class='card'><div class='label'>GYRO (X/Y/Z)</div><div id='gyr'>--</div></div>"
  "<div class='card'><div class='label'>ATTITUDE (P/R)</div><div id='att'>--</div></div>"
"</div>"

"<div class='controls'>"
  "<h3>PILOTAGE (Clavier ZQSD + A)</h3>"
  "<div class='key-row'>"
"    <div class='key' id='kZ' onclick=\"sendCmd('Z')\">Z</div>"
  "</div>"
  "<div class='key-row'>"
"    <div class='key' id='kQ' onclick=\"sendCmd('Q')\">Q</div>"
"    <div class='key' id='kS' onclick=\"sendCmd('S')\">S</div>"
"    <div class='key' id='kD' onclick=\"sendCmd('D')\">D</div>"
  "</div>"
  "<div class='key-row'>"
"    <div class='key' id='kA' onclick=\"sendCmd('A')\">A (AUTO)</div>"
  "</div>"
"</div>"

"</div>"
"<script>"
"function sendCmd(k){fetch('/cmd?key='+k);}"

// visuel touches actives
"function setKeyActive(k,a){"
"  k=k.toUpperCase();"
"  var el=document.getElementById('k'+k);"
"  if(!el)return;"
"  if(a)el.classList.add('active');"
"  else el.classList.remove('active');"
"}"

// clavier ZQSD + A
"window.addEventListener('keydown',function(e){"
"  var k=e.key;"
"  if(!k)return;"
"  k=k.toUpperCase();"
"  if(['Z','Q','S','D','A'].includes(k)){"
"    if(e.repeat)return;"
"    e.preventDefault();"
"    setKeyActive(k,true);"
"    sendCmd(k);"
"  }"
"});"

"window.addEventListener('keyup',function(e){"
"  var k=e.key;"
"  if(!k)return;"
"  k=k.toUpperCase();"
"  if(['Z','Q','S','D','A'].includes(k)){"
"    e.preventDefault();"
"    setKeyActive(k,false);"
"    if(['Z','Q','S','D'].includes(k)){"
"      sendCmd('S');"
"    }"
"  }"
"});"

// rafraîchissement des données
"setInterval(function(){"
"  fetch('/data').then(function(r){return r.json();}).then(function(d){"
"    var el;"
"    el=document.getElementById('vbat'); if(el)el.innerText=d.v.toFixed(2);"
"    el=document.getElementById('prof'); if(el)el.innerText=d.p.toFixed(2);"
"    el=document.getElementById('yaw');  if(el)el.innerText=d.yaw.toFixed(0);"
"    el=document.getElementById('acc');  if(el)el.innerText=d.ax.toFixed(1)+'/'+d.ay.toFixed(1)+'/'+d.az.toFixed(1);"
"    el=document.getElementById('gyr');  if(el)el.innerText=d.gx.toFixed(1)+'/'+d.gy.toFixed(1)+'/'+d.gz.toFixed(1);"
"    el=document.getElementById('att');  if(el)el.innerText=d.pit.toFixed(0)+'/'+d.rol.toFixed(0);"
"    el=document.getElementById('mode'); if(el)el.innerText=d.auto?'AUTONOME':'MANUEL';"
"  });"
"},200);"
"</script>"
"</body></html>"
  ));
}


