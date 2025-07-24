#include <HardwareSerial.h>
#include <AlfredoCRSF.h>
#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "C3-Mini-AP";
const char* password = "12345678";

WiFiUDP udp;
const int udpPort = 8888;

struct ControlData {
  int throttle;
  int steering;
};

ControlData controlData;

HardwareSerial crsfSerialIn(1);  // UART1 RX: GPIO5
AlfredoCRSF crsfIn;

HardwareSerial crsfSerialOut(0); // UART0 TX: GPIO6
AlfredoCRSF crsfOut;

#define PIN_RX_IN 5
#define PIN_TX_OUT 6

void printChannels(const crsf_channels_t& channels) {
  Serial.print(channels.ch0); Serial.print(", ");
  Serial.print(channels.ch1); Serial.print(", ");
  Serial.print(channels.ch2); Serial.print(", ");
  Serial.print(channels.ch3); Serial.print(", ");
  Serial.print(channels.ch4); Serial.print(", ");
  Serial.print(channels.ch5); Serial.print(", ");
  Serial.print(channels.ch6); Serial.print(", ");
  Serial.print(channels.ch7); Serial.print(", ");
  Serial.print(channels.ch8); Serial.print(", ");
  Serial.print(channels.ch9); Serial.print(", ");
  Serial.print(channels.ch10); Serial.print(", ");
  Serial.print(channels.ch11); Serial.print(", ");
  Serial.print(channels.ch12); Serial.print(", ");
  Serial.print(channels.ch13); Serial.print(", ");
  Serial.print(channels.ch14); Serial.print(", ");
  Serial.println(channels.ch15);
}

void setup() {
  Serial.begin(9600);

  delay(1000);

  WiFi.softAP(ssid, password);
  Serial.println("WiFi Access Point started");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  udp.begin(udpPort);
  Serial.printf("UDP server started on port %d\n", udpPort);

  crsfSerialIn.begin(CRSF_BAUDRATE, SERIAL_8N1, PIN_RX_IN, -1);
  crsfIn.begin(crsfSerialIn);

  crsfSerialOut.begin(CRSF_BAUDRATE, SERIAL_8N1, -1, PIN_TX_OUT);
  crsfOut.begin(crsfSerialOut);

  delay(1000);
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    udp.read((uint8_t*)&controlData, sizeof(controlData));
    Serial.print("Received UDP throttle: ");
    Serial.print(controlData.throttle);
    Serial.print(", steering: ");
    Serial.println(controlData.steering);
  }

  crsfIn.update();

  if (crsfIn.isLinkUp()) {
    const crsf_channels_t* inChannels = crsfIn.getChannelsPacked();

    crsf_channels_t outChannels = *inChannels;

    if (controlData.throttle >= 190 && controlData.throttle <= 1811) {
      outChannels.ch3 = controlData.throttle;
    }

    //Serial.print("CRSF Channels (modified): ");
    //printChannels(outChannels);

    crsfOut.writePacket(CRSF_SYNC_BYTE, CRSF_FRAMETYPE_RC_CHANNELS_PACKED, &outChannels, sizeof(outChannels));
  } else {
    Serial.println("Updated");
    delay(500);
  }
}
