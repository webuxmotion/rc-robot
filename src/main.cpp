#include <lilka.h>
#include <WiFi.h>
#include <WiFiUdp.h>

// Налаштування Wi-Fi
const char* ssid = "RCController";
const char* password = "12345678";
const IPAddress receiverIP(192, 168, 4, 1);  // IP адреса C3-mini
const int udpPort = 8888;

WiFiUDP udp;

// Структура даних для керування
struct ControlData {
    int throttle;  // Газ
    int steering;  // Кермо
};

ControlData controlData = {0, 0};  // Початкові значення

void setup() {
    lilka::begin();
    Serial.begin(9600);  // За замовчуванням USB-порт

    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n✅ Connected to WiFi!");
    udp.begin(udpPort);
}

void loop() {
    lilka::State state = lilka::controller.getState();
    const int step = 10;

    // Керування газом (throttle)
    if (state.up.justPressed) {
        controlData.throttle += step;
    } else if (state.down.justPressed) {
        controlData.throttle -= step;
    }

    // Керування поворотом (steering)
    if (state.right.justPressed) {
        controlData.steering += step;
    } else if (state.left.justPressed) {
        controlData.steering -= step;
    }

    // Обмеження значень в межах [0, 1000]
    controlData.throttle = constrain(controlData.throttle, 0, 1000);
    controlData.steering = constrain(controlData.steering, 0, 1000);

    // Вивід у консоль
    Serial.print("Throttle: ");
    Serial.print(controlData.throttle);
    Serial.print(", Steering: ");
    Serial.println(controlData.steering);

    // Надсилання по UDP
    udp.beginPacket(receiverIP, udpPort);
    udp.write((uint8_t*)&controlData, sizeof(controlData));
    udp.endPacket();

    delay(20);  // щоб не перевантажувати UDP-передачу
}
