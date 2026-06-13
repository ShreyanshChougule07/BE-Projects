#include <Adafruit_Fingerprint.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <HTTPClient.h>

// ===== OLED =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ===== Fingerprint =====
HardwareSerial mySerial(2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// ===== WiFi =====
const char* ssid = "SHREYANSH";
const char* password = "20724720";

// ===== Google Script URL =====
String scriptURL = "https://script.google.com/macros/s/AKfycbxac-JZC4F5qBEWGfUeKD81qVQTvKyC716sCAeiPyO3ChD2jL-Ob_HrjX_Gd7JX_aO_7w/exec";

// ===== PINS =====
#define BUZZER 5
#define LECTURE_BTN 4
#define DAY_BTN 15

// ===== Attendance =====
bool marked[11] = {false};
bool showIdle = true;

// ===== NAME FUNCTION =====
String getName(int id) {
  switch(id) {
    case 1: return "Rahul";
    case 2: return "Amit";
    case 3: return "Priya";
    case 4: return "Neha";
    case 5: return "Rohan";
    case 6: return "Arjun";
    case 7: return "Sneha";
    case 8: return "Pooja";
    case 9: return "Vikas";
    case 10: return "Anjali";
    default: return "Unknown";
  }
}

// ===== BUZZER =====
void beep(int duration) {
  digitalWrite(BUZZER, HIGH);
  delay(duration);
  digitalWrite(BUZZER, LOW);
}

// ===== OLED =====
void showMessage(String msg) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 20);
  display.println(msg);
  display.display();
}

// ===== FINGERPRINT =====
int getFingerprintID() {
  int p = finger.getImage();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) return -1;

  return finger.fingerID;
}

// ===== GOOGLE =====
void sendToGoogle(String name, int id) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = scriptURL + "?name=" + name + "&id=" + String(id);
    http.begin(url);
    http.GET();
    http.end();
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(21, 22);

  pinMode(BUZZER, OUTPUT);
  pinMode(LECTURE_BTN, INPUT_PULLUP);
  pinMode(DAY_BTN, INPUT_PULLUP);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  showMessage("STARTING");

  // WiFi
  WiFi.begin(ssid, password);
  showMessage("CONNECTING");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  showMessage("WIFI OK");
  delay(1500);

  // Fingerprint
  mySerial.begin(57600, SERIAL_8N1, 16, 17);
  finger.begin(57600);

  if (finger.verifyPassword()) {
    showMessage("SENSOR OK");
  } else {
    showMessage("ERROR");
    while (1);
  }

  delay(1500);
}

void loop() {

  // ===== NEW LECTURE =====
  if (digitalRead(LECTURE_BTN) == LOW) {

    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);

    display.setCursor(0, 18);
    display.println("NEW");

    display.setCursor(0, 40);
    display.println("LECTURE");

    display.display();

    beep(300);

    for (int i = 0; i < 11; i++) marked[i] = false;

    delay(300);
    sendToGoogle("NEW_LECTURE", 0);

    delay(1500);
    showIdle = true;

    while (digitalRead(LECTURE_BTN) == LOW);
  }

  
  if (digitalRead(DAY_BTN) == LOW) {

    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);

    display.setCursor(10, 25);   // centered
    display.println("NEW DAY");

    display.display();

    beep(500);

    for (int i = 0; i < 11; i++) marked[i] = false;

    delay(300);
    sendToGoogle("NEW_DAY", 0);

    delay(1500);
    showIdle = true;

    while (digitalRead(DAY_BTN) == LOW);
  }

  // ===== IDLE =====
  if (showIdle) {
    showMessage("PLACE THE FINGER");
    showIdle = false;
  }

  int id = getFingerprintID();

  if (id != -1) {

    String name = getName(id);

    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);

    display.setCursor(0, 0);
    display.println("WELCOME");

    display.setCursor(0, 20);
    display.println(name);

    if (id >= 1 && id <= 10) {

      if (!marked[id]) {

        display.setCursor(0, 45);
        display.println("MARKED");
        marked[id] = true;

        beep(200);
        display.display();

        delay(300);
        sendToGoogle(name, id);

      } else {
        display.setCursor(0, 45);
        display.println("ALREADY");
        beep(800);
        display.display();
      }

    } else {
      beep(1000);
    }

    delay(2000);
    showIdle = true;
  }

  delay(50);
}