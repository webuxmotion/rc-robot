#include <lilka.h>
#include <WiFi.h>
#include <WiFiUdp.h>

// Wi-Fi налаштування
const char *ssid = "C3-Mini-AP";
const char *password = "12345678";
const char *c3MiniIP = "192.168.4.1";
const int udpPort = 8888;

WiFiUDP udp;

// Дані, які надсилаємо
struct ControlData
{
  int throttle;
};

ControlData controlData = {340}; // Початковий газ

// Межі
const int minThrottle = 340;
const int maxThrottle = 1811;
const int step = 2;

void setup()
{
  lilka::begin();
  Serial.begin(9600);

  // Підключення до Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
}

void loop()
{
  lilka::State state = lilka::controller.getState();

  // Плавна зміна throttle
  if (state.up.pressed)
  {
    controlData.throttle += step;
  }
  if (state.down.pressed)
  {
    controlData.throttle -= step;
  }

  controlData.throttle = constrain(controlData.throttle, minThrottle, maxThrottle);

  // Вивід в монітор порту
  Serial.print("Throttle: ");
  Serial.println(controlData.throttle);

  // Відображення на Lilka дисплеї
  lilka::display.fillScreen(lilka::colors::Black); // Заповнити екран червоним кольором
  lilka::display.setCursor(20, 100);
  lilka::display.setTextColor(lilka::colors::White); // Зелений текст
  lilka::display.setTextSize(2);
  lilka::display.print(controlData.throttle);

  // Надсилання даних на C3-Mini
  udp.beginPacket(c3MiniIP, udpPort);
  udp.write((uint8_t *)&controlData, sizeof(controlData));
  udp.endPacket();

  delay(20);
}
