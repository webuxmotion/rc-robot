#include <HardwareSerial.h>
#include <AlfredoCRSF.h>
#include <WiFi.h>
#include <WiFiUdp.h>

const char *ssid = "C3-Mini-AP";
const char *password = "12345678";

#define TX12_MIN 190
#define TX12_MAX 1811

WiFiUDP udp;
const int udpPort = 8888;

struct ControlData
{
  int throttle;
  int steering;
};

ControlData controlData;

HardwareSerial crsfSerialIn(1); // UART1 RX: GPIO5
AlfredoCRSF crsfIn;

HardwareSerial crsfSerialOut(0); // UART0 TX: GPIO6
AlfredoCRSF crsfOut;

#define PIN_RX_IN 5
#define PIN_TX_OUT 6

int convertCh(int chValue) {
  return map(chValue, TX12_MIN, TX12_MAX, CRSF_CHANNEL_VALUE_MIN, CRSF_CHANNEL_VALUE_2000);
}

void printChannels(const crsf_channels_t &channels)
{
  Serial.print(channels.ch0);
  Serial.print(", ");
  Serial.print(channels.ch1);
  Serial.print(", ");
  Serial.print(channels.ch2);
  Serial.print(", ");
  Serial.print(channels.ch3);
  Serial.print(", ");
  Serial.print(channels.ch4);
  Serial.print(", ");
  Serial.print(channels.ch5);
  Serial.print(", ");
  Serial.print(channels.ch6);
  Serial.print(", ");
  Serial.print(channels.ch7);
  Serial.print(", ");
  Serial.print(channels.ch8);
  Serial.print(", ");
  Serial.print(channels.ch9);
  Serial.print(", ");
  Serial.print(channels.ch10);
  Serial.print(", ");
  Serial.print(channels.ch11);
  Serial.print(", ");
  Serial.print(channels.ch12);
  Serial.print(", ");
  Serial.print(channels.ch13);
  Serial.print(", ");
  Serial.print(channels.ch14);
  Serial.print(", ");
  Serial.println(channels.ch15);
}

void setup()
{
  Serial.begin(9600);

  delay(300);

  crsfSerialOut.begin(CRSF_BAUDRATE, SERIAL_8N1, -1, PIN_TX_OUT);
  crsfOut.begin(crsfSerialOut);

  delay(300);

  crsfSerialIn.begin(CRSF_BAUDRATE, SERIAL_8N1, PIN_RX_IN, -1);
  crsfIn.begin(crsfSerialIn);

  delay(300);

  WiFi.softAP(ssid, password);

  delay(2000);

  Serial.println("WiFi Access Point started");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  udp.begin(udpPort);

  delay(2000);

  Serial.printf("UDP server started on port %d\n", udpPort);
}

void loop()
{
  crsfIn.update();

  int packetSize = udp.parsePacket();

  if (packetSize)
  {
    udp.read((uint8_t *)&controlData, sizeof(controlData));
  }

  if (crsfIn.isLinkUp())
  {
    const crsf_channels_t *inChannels = crsfIn.getChannelsPacked();

    crsf_channels_t outChannels = *inChannels;

    if (controlData.throttle >= TX12_MIN && controlData.throttle <= TX12_MAX)
    {
      outChannels.ch2 = convertCh(controlData.throttle);

      Serial.println("Throttle updated");

      //printChannels(outChannels);
    } else {
      printChannels(outChannels);
    }

    crsfOut.writePacket(CRSF_SYNC_BYTE, CRSF_FRAMETYPE_RC_CHANNELS_PACKED, &outChannels, sizeof(outChannels));
  }
  else
  {
    Serial.println("Updated");
    delay(500);
  }
}
