#include "WiFiText.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Arduino.h>

// ===== Buttons (exit combo) =====
#define BTN_UP     25
#define BTN_DOWN   26

// ===== Display =====
static Adafruit_SH1106G* display;

// ===== Web Server =====
static WebServer server(80);

// ===== State =====
// Change: Initializing an empty string to store multiple messages
static String receivedText = ""; 
static bool showText = false;

// ===== Draw WiFi Scan =====
static void drawWiFiScan() {
  display->clearDisplay();
  display->setTextSize(1);
  display->setCursor(0, 0);
  display->println("WiFi Scan:");

  int n = WiFi.scanNetworks();
  for (int i = 0; i < n && i < 5; i++) {
    display->setCursor(0, 10 + i * 10);
    display->print(WiFi.SSID(i));
    display->print(" ");
    display->print(WiFi.RSSI(i));
  }

  display->display();
}

// ===== Draw Text =====
static void drawText() {
  display->clearDisplay();
  display->setTextSize(1);
  display->setCursor(0, 0);
  // Change: Printing the whole history instead of just one line
  display->println(receivedText);
  display->display();
}

// ===== Web Page =====
static void handleRoot() {
  String page =
    "<html><body>"
    "<h3>ESP32 OLED</h3>"
    "<form action='/send'>"
    "<input name='msg' type='text'>"
    "<input type='submit' value='Send'>"
    "</form>"
    "</body></html>";

  server.send(200, "text/html", page);
}

// ===== Receive Text =====
static void handleSend() {
  if (server.hasArg("msg")) {
    // Change: Get Device ID (IP) and append new message to the old history
    String ip = server.client().remoteIP().toString();
    receivedText += ip + ": " + server.arg("msg") + "\n";
    
    // Optional: Simple trim to keep memory safe if history gets huge
    if (receivedText.length() > 300) receivedText = receivedText.substring(100);

    showText = true;
    drawText();
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

// ===== Init =====
void wifiTextInit(Adafruit_SH1106G* disp) {
  display = disp;

  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);

  // Start WiFi ONLY for this app
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("ESP32-WiFiApp");   // 🔓 no password

  WiFi.scanNetworks();
  drawWiFiScan();

  server.on("/", handleRoot);
  server.on("/send", handleSend);
  server.begin();
}

// ===== Loop =====
bool wifiTextLoop() {

  // Exit app (UP + DOWN)
  if (digitalRead(BTN_UP) == LOW &&
      digitalRead(BTN_DOWN) == LOW) {
    delay(300);

    server.stop();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);

    return true;
  }

  server.handleClient();
  return false;
}