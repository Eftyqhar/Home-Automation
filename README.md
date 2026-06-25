# Smart Home Automation — ESP32 + 4-Channel Relay

Control 4 relays via Google Home (voice), mobile web remote, and physical switches.
Channels 1–3 use SinricPro (cloud). Channel 4 uses Espalexa (local LAN).

---

## What You Need

**Hardware**
- ESP32 NodeMCU
- 4-channel relay module
- 4 toggle switches
- Jumper wires

**Accounts**
- [sinric.pro](https://sinric.pro) — free account (3 devices)
- Google account (for Google Home)

---

## Wiring

```
ESP32 NodeMCU          4-Channel Relay Module
─────────────          ──────────────────────
GPIO 26       ──────►  IN1
GPIO 27       ──────►  IN2
GPIO 14       ──────►  IN3
GPIO 12       ──────►  IN4
GND           ──────►  GND
VIN (5V)      ──────►  VCC

ESP32 NodeMCU          Toggle Switches (x4)
─────────────          ───────────────────
GPIO 13       ──┬──►  Switch 1 — pin A
                └──►  GND
GPIO  4       ──┬──►  Switch 2 — pin A
                └──►  GND
GPIO 18       ──┬──►  Switch 3 — pin A
                └──►  GND
GPIO 19       ──┬──►  Switch 4 — pin A
                └──►  GND
```
> Internal PULLUP is enabled — no external resistors needed.

**Relay load wiring (per channel)**
```
Live ──► COM
         NO ──► Load (bulb/fan) ──► Neutral
```

---

## Step 1 — Arduino IDE Setup

1. Install **Arduino IDE** from [arduino.cc](https://www.arduino.cc/en/software)
2. Open Arduino IDE → **File** → **Preferences** → paste this into *Additional Boards Manager URLs*:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Go to **Tools** → **Board** → **Boards Manager** → search `esp32` → install **esp32 by Espressif**
4. Go to **Tools** → **Manage Libraries** → install these one by one:
   - `SinricPro` by Boris Jaeger
   - `ArduinoJson` by Benoit Blanchon
   - `WebSockets` by Markus Sattler
   - `Espalexa` by Fabian Affolter

---

## Step 2 — SinricPro Setup

1. Go to [sinric.pro](https://sinric.pro) → Sign Up
2. After login, go to **Devices** → **Add Device** → repeat 3 times:
   - Type: **Switch**
   - Name them whatever you want (e.g. `Living Room Light`, `Fan`, `Bedroom Light`)
3. Go to **Credentials** → copy your **App Key** and **App Secret**
4. Click each device → copy its **Device ID**

---

## Step 3 — Configure the Code

Open `Smart_Home.ino` and fill in the top section:

```cpp
#define WIFI_SSID    "your_wifi_name"
#define WIFI_PASS    "your_wifi_password"

#define APP_KEY      "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"   // from SinricPro Credentials
#define APP_SECRET   "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"   // from SinricPro Credentials

#define DEVICE_ID_1  "xxxxxxxxxxxxxxxxxxxxxxxx"   // Device ID of Switch 1
#define DEVICE_ID_2  "xxxxxxxxxxxxxxxxxxxxxxxx"   // Device ID of Switch 2
#define DEVICE_ID_3  "xxxxxxxxxxxxxxxxxxxxxxxx"   // Device ID of Switch 3
```

Optionally rename the device labels shown in the web remote:
```cpp
#define DEVICE_NAME_1  "Living Room Light"
#define DEVICE_NAME_2  "Fan"
#define DEVICE_NAME_3  "Bedroom Light"
#define DEVICE_NAME_4  "Switch 4"
```

---

## Step 4 — Flash to ESP32

1. Plug ESP32 into your PC via USB
2. In Arduino IDE → **Tools** → **Board** → select **ESP32 Dev Module**
3. **Tools** → **Port** → select the correct COM port
4. Click **Upload** (arrow button)
5. Open **Serial Monitor** (baud rate `115200`) — you will see the ESP32's IP address printed on boot:
   ```
   Web remote: http://192.168.1.xxx:8080
   ```
   Save this IP address — you'll use it for the mobile web remote.

---

## Step 5 — Google Home Setup

### Channels 1–3 (SinricPro)
1. Open **Google Home** app → tap **+** → **Set up device** → **Works with Google**
2. Search for `SinricPro` → sign in with your SinricPro account
3. Your 3 devices will appear — assign them to rooms

### Channel 4 (Espalexa — local Hue bridge)
1. Make sure ESP32 is powered on and connected to WiFi
2. Open **Google Home** app → tap **+** → **Set up device** → **Works with Google**
3. Search for `Philips Hue` → tap it
4. Google Home will scan your LAN and discover the ESP32 as a Hue bridge
5. `Switch 4` (or whatever you set as `DEVICE_NAME_4`) will appear — assign it to a room

---

## Usage

### Voice Control (Google Home)
```
"Hey Google, turn on the Living Room Light"
"Hey Google, turn off the Fan"
"Hey Google, turn off all lights"
```

### Mobile Web Remote (online or offline)
1. Make sure your phone is on the same WiFi as the ESP32
2. Open any browser and go to:
   ```
   http://<ESP32-IP>:8080
   ```
3. You will see ON/OFF buttons for all 4 channels — tap to control

> Works without internet. As long as your router is on, the web remote works.

### Physical Switches
Press any toggle switch to flip that relay on or off instantly.
If online, the new state is synced to SinricPro automatically.

---

## Channel Summary

| Channel | Relay | Switch | Google Home | Web Remote |
|---------|-------|--------|-------------|------------|
| 1 | GPIO 26 | GPIO 13 | ✅ SinricPro (cloud) | ✅ |
| 2 | GPIO 27 | GPIO 4  | ✅ SinricPro (cloud) | ✅ |
| 3 | GPIO 14 | GPIO 18 | ✅ SinricPro (cloud) | ✅ |
| 4 | GPIO 12 | GPIO 19 | ✅ Espalexa (LAN)    | ✅ |

---

## Offline Behavior

| Feature | No Internet |
|---------|-------------|
| Physical switches | ✅ Always work |
| Web remote (`http://<IP>:8080`) | ✅ Works on LAN |
| Google Home voice | ❌ Needs internet |
| SinricPro app | ✅ Works on LAN |
| State sync to cloud | ✅ Auto-syncs on reconnect |
