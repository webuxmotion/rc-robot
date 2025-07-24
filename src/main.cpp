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
  int roll;
  int armed;
};

ControlData controlData = {0, 0, 0};

HardwareSerial crsfSerialIn(1); // RX: GPIO5
AlfredoCRSF crsfIn;

HardwareSerial crsfSerialOut(0); // TX: GPIO6
AlfredoCRSF crsfOut;

#define PIN_RX_IN 5
#define PIN_TX_OUT 6

unsigned long lastUdpTime = 0;
const unsigned long UDP_TIMEOUT = 1000; // 1 секунда

bool lilkaActive = false;
bool prevLilkaState = false;

int convertCh(int chValue)
{
  return map(chValue, TX12_MIN, TX12_MAX, CRSF_CHANNEL_VALUE_MIN, CRSF_CHANNEL_VALUE_2000);
}

void setup()
{
  Serial.begin(9600);
  delay(100);

  crsfSerialOut.begin(CRSF_BAUDRATE, SERIAL_8N1, -1, PIN_TX_OUT);
  crsfOut.begin(crsfSerialOut);
  delay(300);

  crsfSerialIn.begin(CRSF_BAUDRATE, SERIAL_8N1, PIN_RX_IN, -1);
  crsfIn.begin(crsfSerialIn);
  delay(300);

  WiFi.softAP(ssid, password);
  delay(100);

  Serial.println("WiFi Access Point started");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  udp.begin(udpPort);
  delay(100);
  Serial.printf("UDP server started on port %d\n", udpPort);
}

void loop()
{
  crsfIn.update();

  // Прийом UDP
  int packetSize = udp.parsePacket();
  if (packetSize)
  {
    udp.read((uint8_t *)&controlData, sizeof(controlData));
    lastUdpTime = millis();
  }

  lilkaActive = (millis() - lastUdpTime < UDP_TIMEOUT);

  if (lilkaActive != prevLilkaState)
  {
    if (lilkaActive)
      Serial.println("✅ Lilka connected!");
    else
      Serial.println("⚠️ Lilka disconnected, fallback");
    prevLilkaState = lilkaActive;
  }

  static crsf_channels_t lastOutChannels;

  crsf_channels_t baseChannels;

  if (crsfIn.isLinkUp())
  {
    // Якщо CRSF-вхід активний, беремо канали з нього
    const crsf_channels_t *inChannels = crsfIn.getChannelsPacked();
    baseChannels = *inChannels;
  }
  else
  {
    // Якщо немає CRSF-входу, беремо останні відомі або нейтральні
    if (lastOutChannels.ch0 == 0 && lastOutChannels.ch2 == 0 && lastOutChannels.ch4 == 0)
    {
      baseChannels.ch0 = 1500;
      baseChannels.ch1 = 400;
      baseChannels.ch2 = 400;
      baseChannels.ch3 = 400;
      baseChannels.ch4 = 1000; // disarm
      baseChannels.ch5 = 1000;
      baseChannels.ch6 = 1000;
      baseChannels.ch7 = 1000;
    }
    else
    {
      baseChannels = lastOutChannels;
    }
  }

  // Якщо Lilka активна, оновлюємо певні канали з UDP даних
  if (lilkaActive)
  {
    if (controlData.roll >= TX12_MIN && controlData.roll <= TX12_MAX)
      baseChannels.ch0 = convertCh(controlData.roll);

    if (controlData.throttle >= TX12_MIN && controlData.throttle <= TX12_MAX)
      baseChannels.ch2 = convertCh(controlData.throttle);

    if (controlData.armed >= TX12_MIN && controlData.armed <= TX12_MAX)
      baseChannels.ch4 = convertCh(controlData.armed);
  }

  // Запам'ятовуємо канал для наступної ітерації
  lastOutChannels = baseChannels;

  // Відправляємо CRSF пакет завжди
  crsfOut.writePacket(CRSF_SYNC_BYTE, CRSF_FRAMETYPE_RC_CHANNELS_PACKED, &baseChannels, sizeof(baseChannels));
}
