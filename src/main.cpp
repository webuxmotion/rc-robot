#include <WiFi.h>
#include <WiFiUdp.h>

// Налаштування Wi-Fi як точки доступу (Access Point)
const char* ssid = "C3-Mini-AP";
const char* password = "12345678";

WiFiUDP udp;
const int udpPort = 8888;

struct ControlData {
  int throttle;
  int steering;
};

ControlData controlData;

void setup() {
  Serial.begin(9600);

  WiFi.softAP(ssid, password);
  Serial.println("WiFi Access Point started");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  udp.begin(udpPort);
  Serial.printf("UDP server started on port %d\n", udpPort);
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    udp.read((uint8_t*)&controlData, sizeof(controlData));
    Serial.print("Received throttle: ");
    Serial.print(controlData.throttle);
    Serial.print(", steering: ");
    Serial.println(controlData.steering);
    // Тут можна додати керування на основі controlData
  }
}
