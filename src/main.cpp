#include <Arduino.h>
#include <AlfredoCRSF.h>
#include <HardwareSerial.h>

#include <WiFi.h>
#include <WiFiUdp.h>

// Налаштування Wi-Fi
const char* ssid = "RCController";  // Назва Wi-Fi мережі пульта
const char* password = "12345678";  // Пароль до мережі пульта

// Налаштування UDP
WiFiUDP udp;
const int udpPort = 8888;  // Порт для прийому даних

// Структура для отримання даних
struct ControlData {
  int throttle;  // Газ
  int steering;  // Кермо
};

ControlData controlData;


#define PIN_RX_OUT 6
#define PIN_TX_OUT 6

#define PIN_RX 5
#define PIN_TX 5

#define TX12_MIN 990
#define TX12_MAX 1998
#define TX12_MID 1500

HardwareSerial crsfSerialOut(0);
AlfredoCRSF crsfOut;

// Set up a new Serial object
HardwareSerial crsfSerial(1);
AlfredoCRSF crsf;

int channelValue = CRSF_CHANNEL_VALUE_MID;

int gear = 0;

int prevPitch = 0;
int prevPitchStartMillis = 0;

// Method to send channels based on CRSF instance
void sendChannels(AlfredoCRSF& crsf) {
  const crsf_channels_t* channels_ptr = crsf.getChannelsPacked();
  crsfOut.writePacket(CRSF_SYNC_BYTE, CRSF_FRAMETYPE_RC_CHANNELS_PACKED, channels_ptr, sizeof(*channels_ptr));
}

// Fallback method to send default channel values
void sendFallbackChannels() {
  crsf_channels_t crsfChannels = { 0 };
  crsfChannels.ch0 = CRSF_CHANNEL_VALUE_MIN;
  crsfChannels.ch1 = CRSF_CHANNEL_VALUE_MIN;
  crsfChannels.ch2 = CRSF_CHANNEL_VALUE_MIN;
  crsfChannels.ch3 = CRSF_CHANNEL_VALUE_MIN;
  crsfChannels.ch4 = CRSF_CHANNEL_VALUE_MIN;
  crsfChannels.ch5 = CRSF_CHANNEL_VALUE_MIN;
  crsfChannels.ch6 = CRSF_CHANNEL_VALUE_MIN;
  crsfChannels.ch7 = CRSF_CHANNEL_VALUE_MIN;
  crsfChannels.ch8 = CRSF_CHANNEL_VALUE_MIN;
  crsfChannels.ch9 = CRSF_CHANNEL_VALUE_MIN;
  crsfChannels.ch10 = CRSF_CHANNEL_VALUE_MIN;
  crsfChannels.ch11 = CRSF_CHANNEL_VALUE_MIN;
  crsfChannels.ch12 = CRSF_CHANNEL_VALUE_MIN;
  crsfChannels.ch13 = CRSF_CHANNEL_VALUE_MIN;
  crsfChannels.ch14 = CRSF_CHANNEL_VALUE_MIN;
  crsfChannels.ch15 = CRSF_CHANNEL_VALUE_MIN;

  crsfOut.writePacket(CRSF_SYNC_BYTE, CRSF_FRAMETYPE_RC_CHANNELS_PACKED, &crsfChannels, sizeof(crsfChannels));
}

// Fallback method to send default channel values
void sendModifiedChannels(
  int roll,
  int pitch,
  int yaw,
  int throttle,
  int ch5_E,
  int ch6_F,
  int ch7_B,
  int ch8_C,
  int ch9_A,
  int ch10_D,
  int ch11_S1,
  int ch12_S2
) {
  crsf_channels_t crsfChannels = { 0 };
  crsfChannels.ch0 = roll;
  crsfChannels.ch1 = pitch;
  crsfChannels.ch2 = yaw;
  crsfChannels.ch3 = throttle;
  crsfChannels.ch4 = ch5_E;
  crsfChannels.ch5 = ch6_F;
  crsfChannels.ch6 = ch7_B;
  crsfChannels.ch7 = ch8_C;
  crsfChannels.ch8 = ch9_A;
  crsfChannels.ch9 = ch10_D;
  crsfChannels.ch10 = ch11_S1;
  crsfChannels.ch11 = ch12_S2;
  crsfChannels.ch12 = CRSF_CHANNEL_VALUE_MIN;
  crsfChannels.ch13 = CRSF_CHANNEL_VALUE_MIN;
  crsfChannels.ch14 = CRSF_CHANNEL_VALUE_MIN;
  crsfChannels.ch15 = CRSF_CHANNEL_VALUE_MIN;

  crsfOut.writePacket(CRSF_SYNC_BYTE, CRSF_FRAMETYPE_RC_CHANNELS_PACKED, &crsfChannels, sizeof(crsfChannels));
}

void relayOn() {
  digitalWrite(1, LOW);
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  digitalWrite(7, LOW);
  digitalWrite(8, LOW);
  digitalWrite(9, LOW);
  digitalWrite(10, LOW);
}

void relayOff() {
  digitalWrite(1, HIGH);
  digitalWrite(2, HIGH);
  digitalWrite(3, HIGH);
  digitalWrite(4, HIGH);
  digitalWrite(7, HIGH);
  digitalWrite(8, HIGH);
  digitalWrite(9, HIGH);
  digitalWrite(10, HIGH);
}

void setup() {
  Serial.begin(9600);

   pinMode(1, OUTPUT);
   pinMode(2, OUTPUT);
   pinMode(3, OUTPUT);
   pinMode(4, OUTPUT);
   pinMode(7, OUTPUT);
   pinMode(8, OUTPUT);
   pinMode(9, OUTPUT);
   pinMode(10, OUTPUT);

   relayOff();

  crsfSerialOut.begin(CRSF_BAUDRATE, SERIAL_8N1, PIN_RX_OUT, PIN_TX_OUT);
  crsfOut.begin(crsfSerialOut);

  delay(30);

  crsfSerial.begin(CRSF_BAUDRATE, SERIAL_8N1, PIN_RX, PIN_TX);
  crsf.begin(crsfSerial);

  delay(300);

  // Підключення до точки доступу пульта
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!");

  // Підготовка UDP
  udp.begin(udpPort);
}

int convertCh(int chValue) {
  return map(chValue, TX12_MIN, TX12_MAX, CRSF_CHANNEL_VALUE_MIN, CRSF_CHANNEL_VALUE_2000);
}



void loop() {
  crsf.update();

  int packetSize = udp.parsePacket();
  if (packetSize) {
    // Отримання даних з пульта
    udp.read((uint8_t*)&controlData, sizeof(controlData));

    // Виведення даних для монітору
    Serial.print("Throttle: ");
    Serial.print(controlData.throttle);

    Serial.print(", Steering: ");
    Serial.println(controlData.steering);

    // Тут можна додати код для керування мотором або сервоприводом
  }

  //Serial.println(crsf.isLinkUp());

  int roll = convertCh(crsf.getChannel(1));
  int pitch = convertCh(crsf.getChannel(2));
  int yaw = convertCh(crsf.getChannel(3));
  int yaw_out = yaw;
  int throttle = convertCh(crsf.getChannel(4));

  int ch5_E = convertCh(crsf.getChannel(5));
  int ch6_F = convertCh(crsf.getChannel(6));
  int ch7_B = convertCh(crsf.getChannel(7));
  int ch8_C = convertCh(crsf.getChannel(8));
  int ch9_A = convertCh(crsf.getChannel(9));
  int ch10_D = convertCh(crsf.getChannel(10));
  int ch11_S1 = convertCh(crsf.getChannel(11));
  int ch12_S2 = convertCh(crsf.getChannel(12));

  if (ch8_C > 1600) {
    relayOn();
  } else {
    relayOff();
  }

  // auto transmition
  boolean autoTransmition = false;

  if (ch6_F > 1600) {
    autoTransmition = true;
  } else {
    autoTransmition = false;
  }
  // END. auto transmition

  // Serial.print("throttle: ");
  // Serial.print(yaw);
  // Serial.print("| roll: ");
  // Serial.print(roll);
  // Serial.print("| Pitch: ");
  // Serial.print(pitch);
  // Serial.print("| prevPitch: ");
  // Serial.print(prevPitch);
  // Serial.print("| gear: ");
  // Serial.print(gear);
  // Serial.print("| prevPitchStartMillis : ");
  // Serial.print(prevPitchStartMillis );

  // Serial.println("");

  if (autoTransmition) {

    if (gear == 0) {
      yaw_out = 100;
    } else if (gear == 1) {
      yaw_out = 375;
    } else if (gear == 2) {
      yaw_out = 385;
    } else if (gear == 3) {
      yaw_out = 395;
    } else {
      yaw_out = 206;
    }

    
    if (pitch > 1100 && prevPitch == 0) {
      prevPitch = pitch;
      prevPitchStartMillis = millis();
    } else if (prevPitch && pitch > 980 && pitch < 1100) {
      if (millis() - prevPitchStartMillis < 300) {
        if (gear < 3) {
          gear++;
        }
      }

      prevPitch = 0;
      prevPitchStartMillis = 0;
    } else if (pitch < 850 && pitch > 251 && prevPitch == 0) {
      prevPitch = pitch;
      prevPitchStartMillis = millis();
    } else if (prevPitch && pitch > 980 && pitch < 1100) {
      if (millis() - prevPitchStartMillis < 300) {
        if (gear > 0) {
          gear--;
        }
      }

      prevPitch = 0;
      prevPitchStartMillis = 0;
    } else if (pitch < 250) {
      gear = 0;
    }

  } else {
    prevPitch = 0;
    prevPitchStartMillis = 0;
    gear = 0;
  }

  if (controlData.throttle == 1000) {
    yaw_out = 375;
  }

  if (crsf.isLinkUp()) {
      sendModifiedChannels(
        roll,
        pitch,
        yaw_out,
        throttle,
        ch5_E,
        ch6_F,
        ch7_B,
        ch8_C,
        ch9_A,
        ch10_D,
        ch11_S1,
        ch12_S2
      );
  } else {
    gear = 0;
    sendFallbackChannels();
  }
}
