#include <lilka.h>
#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "C3-Mini-AP";
const char* password = "12345678";

const char* c3MiniIP = "192.168.4.1";  // IP точки доступу C3-Mini
const int udpPort = 8888;

WiFiUDP udp;

struct ControlData {
  int throttle;
  int steering;
};

ControlData controlData = {0, 0};

void setup() {
  lilka::begin();
  Serial.begin(9600);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
}

void loop() {
  lilka::State state = lilka::controller.getState();
  const int step = 10;

  if (state.up.justPressed) {
    controlData.throttle += step;
  } 
  if (state.down.justPressed) {
    controlData.throttle -= step;
  }
  if (state.right.justPressed) {
    controlData.steering += step;
  }
  if (state.left.justPressed) {
    controlData.steering -= step;
  }

  controlData.throttle = constrain(controlData.throttle, 0, 1000);
  controlData.steering = constrain(controlData.steering, 0, 1000);

  Serial.print("Throttle: ");
  Serial.print(controlData.throttle);
  Serial.print(", Steering: ");
  Serial.println(controlData.steering);

  udp.beginPacket(c3MiniIP, udpPort);
  udp.write((uint8_t*)&controlData, sizeof(controlData));
  udp.endPacket();

  delay(20);
}
