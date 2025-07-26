
#include <lilka.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

const char *apSSID = "Lilka-AP";
const char *apPassword = "12345678";

const char *staSSID = "C3-Mini-AP";
const char *staPassword = "12345678";

const int udpPort = 8888;

WiFiUDP udp;
WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

IPAddress c3MiniIP;
String apIPText = "0.0.0.0";
String staIPText = "0.0.0.0";

struct ControlData {
  int throttle;
  int roll;
  int armed;
};

ControlData controlData = {340, 1000, 340};

const int minThrottle = 340;
const int maxThrottle = 1811;
const int minRoll = 190;
const int maxRoll = 2000;
const int minArmedValue = 340;
const int maxArmedValue = 1800;

void handleRoot() {
  String html = R"rawliteral(
    <html><head><meta name='viewport' content='width=device-width, initial-scale=1'>
    <style>
      body { margin:0; background:#111; color:#fff; font-family:sans-serif; }
      canvas { display:block; margin:20px auto; background:#333; border-radius:12px; }
    </style></head><body>
    <h2 style="text-align:center;">Lilka Canvas Control</h2>
    <canvas id="controlCanvas" width="400" height="400"></canvas>
    <script>
      const canvas = document.getElementById('controlCanvas');
      const ctx = canvas.getContext('2d');

      let throttle = 1000;
      let roll = 1000;
      let armed = false;

      const throttleArea = {x: 60, y: 20, width: 60, height: 300};
      const rollArea = {x: 280, y: 20, width: 60, height: 300};
      const armButton = {x: 140, y: 340, width: 120, height: 40};

      let socket = new WebSocket('ws://' + location.hostname + ':81');

      function drawUI() {
        ctx.clearRect(0, 0, 400, 400);

        ctx.fillStyle = '#555';
        ctx.fillRect(throttleArea.x, throttleArea.y, throttleArea.width, throttleArea.height);
        let thrNorm = (throttle - 340) / (1811 - 340);
        let thrY = throttleArea.y + throttleArea.height - thrNorm * throttleArea.height;
        ctx.fillStyle = '#0f0';
        ctx.fillRect(throttleArea.x, thrY - 10, throttleArea.width, 20);

        ctx.fillStyle = '#555';
        ctx.fillRect(rollArea.x, rollArea.y, rollArea.width, rollArea.height);
        let rollNorm = (roll - 190) / (2000 - 190);
        let rollY = rollArea.y + rollArea.height - rollNorm * rollArea.height;
        ctx.fillStyle = '#0af';
        ctx.fillRect(rollArea.x, rollY - 10, rollArea.width, 20);

        ctx.fillStyle = armed ? '#d33' : '#777';
        ctx.fillRect(armButton.x, armButton.y, armButton.width, armButton.height);
        ctx.fillStyle = '#fff';
        ctx.font = '20px sans-serif';
        ctx.textAlign = 'center';
        ctx.fillText(armed ? 'DISARM' : 'ARM', armButton.x + armButton.width / 2, armButton.y + 27);
      }

      function sendData() {
        if (socket.readyState === WebSocket.OPEN) {
          socket.send(JSON.stringify({
            throttle: throttle,
            roll: roll,
            armed: armed ? 1800 : 340
          }));
        }
      }

      let activeControl = null;

      canvas.addEventListener('mousedown', e => {
        const rect = canvas.getBoundingClientRect();
        const x = e.clientX - rect.left;
        const y = e.clientY - rect.top;

        if (x >= throttleArea.x && x <= throttleArea.x + throttleArea.width &&
            y >= throttleArea.y && y <= throttleArea.y + throttleArea.height) {
          activeControl = 'throttle';
        } else if (x >= rollArea.x && x <= rollArea.x + rollArea.width &&
                   y >= rollArea.y && y <= rollArea.y + rollArea.height) {
          activeControl = 'roll';
        } else if (x >= armButton.x && x <= armButton.x + armButton.width &&
                   y >= armButton.y && y <= armButton.y + armButton.height) {
          armed = !armed;
          drawUI(); sendData();
        }
      });

      canvas.addEventListener('mousemove', e => {
        if (!activeControl) return;
        const rect = canvas.getBoundingClientRect();
        const y = e.clientY - rect.top;

        if (activeControl === 'throttle') {
          let val = throttleArea.y + throttleArea.height - y;
          val = Math.min(Math.max(val, 0), throttleArea.height);
          throttle = Math.round(340 + val / throttleArea.height * (1811 - 340));
        } else if (activeControl === 'roll') {
          let val = rollArea.y + rollArea.height - y;
          val = Math.min(Math.max(val, 0), rollArea.height);
          roll = Math.round(190 + val / rollArea.height * (2000 - 190));
        }

        drawUI(); sendData();
      });

      window.addEventListener('mouseup', () => {
        activeControl = null;
      });

      canvas.addEventListener('touchstart', e => {
        const rect = canvas.getBoundingClientRect();
        const touch = e.touches[0];
        const x = touch.clientX - rect.left;
        const y = touch.clientY - rect.top;

        if (x >= throttleArea.x && x <= throttleArea.x + throttleArea.width &&
            y >= throttleArea.y && y <= throttleArea.y + throttleArea.height) {
          activeControl = 'throttle';
        } else if (x >= rollArea.x && x <= rollArea.x + rollArea.width &&
                   y >= rollArea.y && y <= rollArea.y + rollArea.height) {
          activeControl = 'roll';
        } else if (x >= armButton.x && x <= armButton.x + armButton.width &&
                   y >= armButton.y && y <= armButton.y + armButton.height) {
          armed = !armed;
          drawUI(); sendData();
        }
        e.preventDefault();
      }, { passive: false });

      canvas.addEventListener('touchmove', e => {
        if (!activeControl) return;
        const rect = canvas.getBoundingClientRect();
        const touch = e.touches[0];
        const y = touch.clientY - rect.top;

        if (activeControl === 'throttle') {
          let val = throttleArea.y + throttleArea.height - y;
          val = Math.min(Math.max(val, 0), throttleArea.height);
          throttle = Math.round(340 + val / throttleArea.height * (1811 - 340));
        } else if (activeControl === 'roll') {
          let val = rollArea.y + rollArea.height - y;
          val = Math.min(Math.max(val, 0), rollArea.height);
          roll = Math.round(190 + val / rollArea.height * (2000 - 190));
        }

        drawUI(); sendData();
        e.preventDefault();
      }, { passive: false });

      window.addEventListener('touchend', () => {
        activeControl = null;
      });

      socket.onopen = () => sendData();
      socket.onclose = () => console.log("WebSocket closed");

      drawUI();
    </script></body></html>
  )rawliteral";

  server.send(200, "text/html", html);
}

void onWebSocketMessage(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT) {
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (!error) {
      controlData.throttle = constrain(doc["throttle"], minThrottle, maxThrottle);
      controlData.roll = constrain(doc["roll"], minRoll, maxRoll);
      controlData.armed = constrain(doc["armed"], minArmedValue, maxArmedValue);
    }
  }
}

void setup() {
  lilka::begin();
  Serial.begin(115200);

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(apSSID, apPassword);
  delay(100);
  apIPText = WiFi.softAPIP().toString();

  WiFi.begin(staSSID, staPassword);
  unsigned long timeout = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - timeout < 5000) delay(100);
  if (WiFi.status() == WL_CONNECTED) {
    c3MiniIP = IPAddress(192,168,4,1);
    staIPText = WiFi.localIP().toString();
  }

  udp.begin(udpPort);
  server.on("/", handleRoot);
  server.begin();
  webSocket.begin();
  webSocket.onEvent(onWebSocketMessage);
}

void loop() {
  server.handleClient();
  webSocket.loop();

  lilka::display.fillScreen(lilka::colors::Black);
  lilka::display.setTextColor(lilka::colors::White);
  lilka::display.setTextSize(1);

  lilka::display.setCursor(10, 20);
  lilka::display.print("AP IP: ");
  lilka::display.print(apIPText);

  lilka::display.setCursor(10, 40);
  lilka::display.print("STA IP: ");
  lilka::display.print(staIPText);

  lilka::display.setCursor(10, 80);
  lilka::display.print("Thr: ");
  lilka::display.print(controlData.throttle);

  lilka::display.setCursor(10, 110);
  lilka::display.print("Roll: ");
  lilka::display.print(controlData.roll);

  lilka::display.setCursor(10, 140);
  lilka::display.print("Arm: ");
  lilka::display.print(controlData.armed);

  if (WiFi.status() == WL_CONNECTED) {
    udp.beginPacket(c3MiniIP, udpPort);
    udp.write((uint8_t *)&controlData, sizeof(controlData));
    udp.endPacket();
  }

  delay(20);
}
