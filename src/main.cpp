#include <WiFi.h>
#include <WebServer.h>
#include <lilka.h>

const char* ssid = "SEVENMONE-Robotics";
const char* password = "12345678";

WebServer server(80);
String ipString;

int y = 100; // початкова позиція тексту

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><title>ESP32 AP</title></head><body>";
  html += "<h1>Welcome to Lilka Wi-Fi AP</h1>";
  html += "<p>Your IP: " + ipString + "</p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

IPAddress local_IP(192, 158, 88, 1);     // your desired IP
IPAddress gateway(192, 158, 88, 1);       // usually same as local_IP
IPAddress subnet(255, 255, 255, 0);      // typical subnet mask

void setup() {
  lilka::begin();

  WiFi.softAPConfig(local_IP, gateway, subnet);

  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  ipString = IP.toString();

  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  server.handleClient(); // обробка запитів

  lilka::Canvas canvas;
  canvas.fillScreen(lilka::colors::Black);
  canvas.setCursor(3, y);
  canvas.setTextColor(lilka::colors::White);
  canvas.setTextSize(1);
  canvas.print(ipString);
  lilka::display.drawCanvas(&canvas);

  y++;
  if (y > 200) y = 100;
}
