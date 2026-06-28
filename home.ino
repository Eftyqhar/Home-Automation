#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "ESP32_Smart_Remote";
const char* password = "password123";

WebServer server(80);

const int relay1 = 26;
const int relay2 = 27;
const int relay3 = 14;
const int relay4 = 25;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
  <title>ESP32 Remote</title>
  <style>
    body {
      background-color: #1a1a1a;
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
      margin: 0;
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      user-select: none;
      -webkit-user-select: none;
    }
    .remote {
      background: linear-gradient(145deg, #232323, #1d1d1d);
      padding: 40px 25px;
      border-radius: 40px;
      box-shadow: 10px 10px 20px #121212, -10px -10px 20px #2a2a2a;
      width: 250px;
      text-align: center;
      border: 1px solid #333;
    }
    h2 {
      color: #888;
      margin-top: 0;
      font-size: 18px;
      letter-spacing: 3px;
      font-weight: 600;
      margin-bottom: 40px;
      text-transform: uppercase;
    }
    .row {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-bottom: 30px;
      padding: 0 10px;
    }
    .label {
      color: #bbb;
      font-size: 18px;
      font-weight: 500;
    }
    .btn {
      width: 65px;
      height: 65px;
      border-radius: 50%;
      border: none;
      cursor: pointer;
      outline: none;
      background: linear-gradient(145deg, #383838, #2f2f2f);
      box-shadow: 5px 5px 10px #161616, -5px -5px 10px #505050;
      color: #ff4d4d;
      font-weight: bold;
      font-size: 14px;
      transition: all 0.2s ease-in-out;
    }
    .btn:active {
      box-shadow: inset 5px 5px 10px #161616, inset -5px -5px 10px #505050;
    }
    .btn.on {
      color: #00ff88;
      box-shadow: inset 5px 5px 10px #161616, inset -5px -5px 10px #505050, 0 0 15px rgba(0, 255, 136, 0.4);
    }
  </style>
</head>
<body>
  <div class="remote">
    <h2>Smart Remote</h2>

    <div class="row">
      <span class="label">CH 1</span>
      <button id="btn1" class="btn" onclick="toggle(1)">OFF</button>
    </div>

    <div class="row">
      <span class="label">CH 2</span>
      <button id="btn2" class="btn" onclick="toggle(2)">OFF</button>
    </div>

    <div class="row">
      <span class="label">CH 3</span>
      <button id="btn3" class="btn" onclick="toggle(3)">OFF</button>
    </div>

    <div class="row">
      <span class="label">CH 4</span>
      <button id="btn4" class="btn" onclick="toggle(4)">OFF</button>
    </div>
  </div>

  <script>
    function toggle(relay) {
      var btn = document.getElementById("btn" + relay);
      var action = btn.classList.contains("on") ? "off" : "on";

      fetch('/r' + relay + '/' + action).then(response => {
        if(response.ok) {
          if(action === "on") {
            btn.classList.add("on");
            btn.innerText = "ON";
          } else {
            btn.classList.remove("on");
            btn.innerText = "OFF";
          }
        }
      });
    }
  </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);

  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);

  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  digitalWrite(relay3, HIGH);
  digitalWrite(relay4, HIGH);

  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/", []() {
    server.send(200, "text/html", index_html);
  });

  server.on("/r1/on", []() { digitalWrite(relay1, LOW); server.send(200, "text/plain", "OK"); });
  server.on("/r1/off", []() { digitalWrite(relay1, HIGH); server.send(200, "text/plain", "OK"); });

  server.on("/r2/on", []() { digitalWrite(relay2, LOW); server.send(200, "text/plain", "OK"); });
  server.on("/r2/off", []() { digitalWrite(relay2, HIGH); server.send(200, "text/plain", "OK"); });

  server.on("/r3/on", []() { digitalWrite(relay3, LOW); server.send(200, "text/plain", "OK"); });
  server.on("/r3/off", []() { digitalWrite(relay3, HIGH); server.send(200, "text/plain", "OK"); });

  server.on("/r4/on", []() { digitalWrite(relay4, LOW); server.send(200, "text/plain", "OK"); });
  server.on("/r4/off", []() { digitalWrite(relay4, HIGH); server.send(200, "text/plain", "OK"); });

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}