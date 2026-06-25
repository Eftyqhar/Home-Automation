#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SinricPro.h>
#include <SinricProSwitch.h>
#include <Espalexa.h>

// ─── WiFi ─────────────────────────────────────────────────
#define WIFI_SSID            "YOUR_WIFI_SSID"
#define WIFI_PASS            "YOUR_WIFI_PASSWORD"

// ─── SinricPro (https://sinric.pro) ───────────────────────
#define APP_KEY              "YOUR_SINRICPRO_APP_KEY"
#define APP_SECRET           "YOUR_SINRICPRO_APP_SECRET"
#define DEVICE_ID_1          "SINRICPRO_DEVICE_ID_SWITCH_1"
#define DEVICE_ID_2          "SINRICPRO_DEVICE_ID_SWITCH_2"
#define DEVICE_ID_3          "SINRICPRO_DEVICE_ID_SWITCH_3"

// ─── Device Names (web UI + Google Home) ──────────────────
#define DEVICE_NAME_1        "Living Room Light"
#define DEVICE_NAME_2        "Fan"
#define DEVICE_NAME_3        "Bedroom Light"
#define DEVICE_NAME_4        "Switch 4"          // Espalexa / ch4 name

// ─── Relay Pins ───────────────────────────────────────────
#define RELAY_1              26
#define RELAY_2              27
#define RELAY_3              14
#define RELAY_4              12
#define RELAY_ON             LOW   // change to HIGH if your relay is active-HIGH
#define RELAY_OFF            HIGH

// ─── Physical Switch Pins (INPUT_PULLUP) ──────────────────
#define SWITCH_1             13
#define SWITCH_2             4
#define SWITCH_3             18
#define SWITCH_4             19

// ─── State ────────────────────────────────────────────────
struct Channel {
    uint8_t     relay;
    uint8_t     sw;
    bool        state;
    bool        lastSwitchState;
    const char* deviceId;    // nullptr = Espalexa (ch4)
    const char* name;
};

// ch0-2: SinricPro (cloud + Google Home + app)
// ch3:   Espalexa  (local Hue bridge → Google Home on LAN)
Channel channels[4] = {
    {RELAY_1, SWITCH_1, false, HIGH, DEVICE_ID_1, DEVICE_NAME_1},
    {RELAY_2, SWITCH_2, false, HIGH, DEVICE_ID_2, DEVICE_NAME_2},
    {RELAY_3, SWITCH_3, false, HIGH, DEVICE_ID_3, DEVICE_NAME_3},
    {RELAY_4, SWITCH_4, false, HIGH, nullptr,      DEVICE_NAME_4},
};

WebServer server(8080);
Espalexa  espalexa;

// ─── Apply relay state ────────────────────────────────────
void applyRelay(int i) {
    digitalWrite(channels[i].relay, channels[i].state ? RELAY_ON : RELAY_OFF);
}

// ─── Sync ch0-2 state to SinricPro cloud ─────────────────
void syncToCloud(int i) {
    if (WiFi.status() != WL_CONNECTED) return;
    SinricProSwitch &sw = SinricPro[channels[i].deviceId];
    sw.sendPowerStateEvent(channels[i].state);
}

// ─── Icons per channel ────────────────────────────────────
const char* ICONS[4] = {"💡", "🌀", "💡", "🔌"};

// ─── Web Remote (port 8080, works offline on LAN) ─────────
void handleRoot() {
    String html =
        "<!DOCTYPE html><html lang='en'><head>"
        "<meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<title>Smart Home</title>"
        "<style>"
          "*{box-sizing:border-box;margin:0;padding:0}"
          "body{font-family:'Segoe UI',sans-serif;background:#0f0f1a;color:#f0f0f0;min-height:100vh;padding:24px 16px}"
          "header{text-align:center;margin-bottom:28px}"
          "header h1{font-size:1.6rem;font-weight:700;letter-spacing:1px;color:#fff}"
          "header p{font-size:.8rem;color:#666;margin-top:4px}"
          ".grid{display:grid;grid-template-columns:1fr 1fr;gap:14px;max-width:420px;margin:auto}"
          ".card{background:#1a1a2e;border-radius:20px;padding:20px 16px;display:flex;flex-direction:column;align-items:center;gap:12px;box-shadow:0 4px 20px rgba(0,0,0,.4);transition:transform .1s}"
          ".card:active{transform:scale(.97)}"
          ".card.active{background:#1e2a1e;box-shadow:0 4px 24px rgba(76,175,80,.25)}"
          ".icon{font-size:2.2rem;line-height:1}"
          ".name{font-size:.85rem;font-weight:600;text-align:center;color:#ccc;letter-spacing:.5px}"
          ".badge{font-size:.7rem;font-weight:700;letter-spacing:1px;padding:3px 10px;border-radius:20px;text-transform:uppercase}"
          ".badge.on{background:#1b5e20;color:#69f06a}"
          ".badge.off{background:#2a1a1a;color:#888}"
          ".toggle{width:64px;height:34px;border-radius:34px;border:none;cursor:pointer;position:relative;transition:background .3s;flex-shrink:0}"
          ".toggle.on{background:#4caf50}"
          ".toggle.off{background:#333}"
          ".toggle::after{content:'';position:absolute;top:4px;width:26px;height:26px;border-radius:50%;background:#fff;transition:left .3s;box-shadow:0 2px 4px rgba(0,0,0,.4)}"
          ".toggle.on::after{left:34px}"
          ".toggle.off::after{left:4px}"
          "footer{text-align:center;margin-top:32px;font-size:.72rem;color:#444}"
        "</style></head>"
        "<body>"
        "<header><h1>&#127968; Smart Home</h1><p>Tap a card to toggle</p></header>"
        "<div class='grid'>";

    for (int i = 0; i < 4; i++) {
        bool on = channels[i].state;
        String act  = on ? " active" : "";
        String tog  = on ? " on" : " off";
        String href = on ? "/off?ch=" : "/on?ch=";
        html += "<a href='" + href + String(i) + "' style='text-decoration:none'>";
        html += "<div class='card" + act + "'>";
        html += "<div class='icon'>" + String(ICONS[i]) + "</div>";
        html += "<div class='name'>" + String(channels[i].name) + "</div>";
        html += "<span class='badge" + tog + "'>" + (on ? "ON" : "OFF") + "</span>";
        html += "<button class='toggle" + tog + "' onclick='event.preventDefault()'></button>";
        html += "</div></a>";
    }

    html += "</div>"
            "<footer>Smart Home &bull; ESP32</footer>"
            "</body></html>";
    server.send(200, "text/html", html);
}

void handleOn() {
    int ch = server.arg("ch").toInt();
    if (ch >= 0 && ch < 4) {
        channels[ch].state = true;
        applyRelay(ch);
        if (ch < 3) syncToCloud(ch);
    }
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleOff() {
    int ch = server.arg("ch").toInt();
    if (ch >= 0 && ch < 4) {
        channels[ch].state = false;
        applyRelay(ch);
        if (ch < 3) syncToCloud(ch);
    }
    server.sendHeader("Location", "/");
    server.send(303);
}

// ─── Espalexa callback (ch4 — Google Home local) ─────────
void onRelay4(uint8_t brightness) {
    channels[3].state = (brightness > 0);
    applyRelay(3);
}

// ─── SinricPro callback (ch0-2 — cloud / Google Home) ────
bool onDeviceState(const String &deviceId, bool &state) {
    for (int i = 0; i < 3; i++) {
        if (deviceId == channels[i].deviceId) {
            channels[i].state = state;
            applyRelay(i);
            return true;
        }
    }
    return false;
}

// ─── WiFi ─────────────────────────────────────────────────
void connectWiFi() {
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print("Connecting WiFi");
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
        delay(300);
        Serial.print(".");
    }
    Serial.println(WiFi.status() == WL_CONNECTED ? "\nWiFi OK" : "\nOffline mode");
}

// ─── Setup ────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);

    for (int i = 0; i < 4; i++) {
        pinMode(channels[i].relay, OUTPUT);
        pinMode(channels[i].sw, INPUT_PULLUP);
        applyRelay(i);
        channels[i].lastSwitchState = digitalRead(channels[i].sw);
    }

    connectWiFi();
    Serial.println("Web remote: http://" + WiFi.localIP().toString() + ":8080");

    server.on("/",    handleRoot);
    server.on("/on",  handleOn);
    server.on("/off", handleOff);
    server.begin();

    for (int i = 0; i < 3; i++) {
        SinricProSwitch &sw = SinricPro[channels[i].deviceId];
        sw.onPowerState(onDeviceState);
    }
    SinricPro.begin(APP_KEY, APP_SECRET);

    espalexa.addDevice(DEVICE_NAME_4, onRelay4);
    espalexa.begin();
}

// ─── Loop ─────────────────────────────────────────────────
void loop() {
    server.handleClient();

    if (WiFi.status() == WL_CONNECTED) {
        SinricPro.handle();
        espalexa.loop();
    }

    for (int i = 0; i < 4; i++) {
        bool current = digitalRead(channels[i].sw);
        if (current == LOW && channels[i].lastSwitchState == HIGH) {
            channels[i].state = !channels[i].state;
            applyRelay(i);
            if (i < 3) syncToCloud(i);
            delay(50);
        }
        channels[i].lastSwitchState = current;
    }
}
