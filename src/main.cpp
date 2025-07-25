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
  int roll;
  int armed; // analog value для армінгу
};

ControlData controlData = {
    340,    // throttle
    1000,   // roll (центр)
    340    // armed, початково роззброєно (disarm)
};

// Межі
const int minThrottle = 340;
const int maxThrottle = 1811;

const int minRoll = 190;
const int maxRoll = 2000;
const int centerRoll = 1000;

const int throttleStep = 3;
const int rollStep = 30;
const int rollReturnStep = 30;

const int minArmedValue = 340; // disarm (мінімум для тумблера TX12)
const int maxArmedValue = 1800; // arm (максимум для тумблера TX12)

bool isArmed = false; // стан армінгу

void setup()
{
  lilka::begin();
  Serial.begin(9600);

  WiFi.begin(ssid, password);
  Serial.println("\nConnected!");
}

void loop()
{
  lilka::State state = lilka::controller.getState();

  // Перемикач arming (toggle)
  if (state.start.justPressed)
  {
    isArmed = !isArmed;
    controlData.armed = isArmed ? maxArmedValue : minArmedValue;
  }

  // Плавна зміна throttle
  if (state.up.pressed)
    controlData.throttle += throttleStep;

  if (state.down.pressed)
    controlData.throttle -= throttleStep;

  // Зміна roll (a = вправо, d = вліво)
  if (state.d.pressed)
  {
    controlData.roll -= rollStep;
  }
  else if (state.a.pressed)
  {
    controlData.roll += rollStep;
  }
  else
  {
    // Автоцентр roll
    if (abs(controlData.roll - centerRoll) <= rollReturnStep)
      controlData.roll = centerRoll;
    else if (controlData.roll > centerRoll)
      controlData.roll -= rollReturnStep;
    else if (controlData.roll < centerRoll)
      controlData.roll += rollReturnStep;
  }

  // Обмеження
  controlData.throttle = constrain(controlData.throttle, minThrottle, maxThrottle);
  controlData.roll = constrain(controlData.roll, minRoll, maxRoll);
  controlData.armed = constrain(controlData.armed, minArmedValue, maxArmedValue);

  // Вивід у Serial
  Serial.print("Throttle: ");
  Serial.print(controlData.throttle);
  Serial.print(" | Roll: ");
  Serial.print(controlData.roll);
  Serial.print(" | Armed: ");
  Serial.println(controlData.armed);

  // Відображення на дисплеї
  lilka::display.fillScreen(lilka::colors::Black);
  lilka::display.setTextColor(lilka::colors::White);
  lilka::display.setTextSize(1);

  lilka::display.setCursor(20, 80);
  lilka::display.print("Thr: ");
  lilka::display.print(controlData.throttle);

  lilka::display.setCursor(20, 120);
  lilka::display.print("Roll: ");
  lilka::display.print(controlData.roll);

  lilka::display.setCursor(20, 160);
  lilka::display.print("Armed: ");
  lilka::display.print(controlData.armed);

  // Надсилання даних
  udp.beginPacket(c3MiniIP, udpPort);
  udp.write((uint8_t *)&controlData, sizeof(controlData));
  udp.endPacket();

  delay(20);
}
