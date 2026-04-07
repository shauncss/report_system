#define BLYNK_TEMPLATE_ID "TMPL65uqZT9wX"
#define BLYNK_TEMPLATE_NAME "report system"
#define BLYNK_AUTH_TOKEN "bI5pSdPO3xxSZ5rmHWUAVOaIMcN2QMC2"

#include <Arduino.h>
#include <WiFi.h>
#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <BlynkSimpleEsp32.h>
#include <ThingSpeak.h>

// WiFi Credentials 
char ssid[] = "Wokwi-GUEST";
char pass[] = ""; // No password needed for Wokwi-GUEST

// Thingspeak constants
const int myChannelNumber =2973762 ;
const char* myApiKey = "73S6VMG4SLJMDTXH";
WiFiClient client;

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Keypad setup
const byte ROWS = 4, COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {19, 18, 5, 17};
byte colPins[COLS] = {16, 4, 0, 2};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Panic alert pins
const int buzzerPin = 25;
const int redLEDPin = 26;
const int panicButtonPin = 35;
const int buttonPin = 34;
int state = 0;
int i = 0;

// Voilence types declaration
const char* violenceTypes[] = {
  "1.Physical", "2.Sexual", "3.Emotion", "4.Digital", "5.Stalking/IPV"
};

// Locations declaration
const char* locations[] = {
  "1.Hospital PP", "2.Tmn Free Schl", "3.Bus Terminal", "4.USM"
};

// Alerts declaration
const char* blynkEvents[] = {
  "physical_alert", "sexual_alert", "emotional_alert", "digital_alert", "stalking_alert"
};

//Blynk messages declaration
const char* blynkMsg[] = {
  "Physical violence alert triggered!", "Sexual violence alert triggered!",
  "Emotional violence alert triggered!", "Digital violence alert triggered!",
  "Stalking violence alert triggered!"
};

const char* responsePages[][4] = {
  { // Physical (index 0)
    "Alert sent to\nlocal emergency",
    "services.\n ",
    "You will\nbe safe!",
    ""
  },
  { // Sexual (index 1)
    "Alert sent to\nKPWKM. Stay safe",
    "You're not alone.\nHelp is coming.",
    "", ""
  },
  { // Emotional (index 2)
    "Call WAO now:\n03 3000 8858",
    "You're not alone.\nHelp is here.",
    "", ""
  },
  { // Digital (index 3)
    "Alert sent to\nCCID",
    "Tips: Change\npasswords often",
    "Enable 2-Factor\nAuthentication",
    "& avoid\nsuspicious links"
  },
  { // Stalking/IPV (index 4)
    "Dial 999 or call\ntrusted contact",
    "Need shelter?\nCall 03-7770 9999",
    "Stay calm.\nYou're not alone.",
    ""
  }
};

void lcdPrint(const char* line1, const char* line2 = "", int delayMs = 0) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
  if (delayMs) delay(delayMs);
}

bool checkExitKey(char key) {
  if (key == 'D') {
    lcdPrint("Exiting...", "", 1000);
    lcd.clear();
    i = 0;
    state = 0;
    return true;
  }
  return false;
}

void panicMode() {
  lcdPrint("! PANIC ALERT !", "Help is OTW!");
  digitalWrite(buzzerPin, HIGH);
  digitalWrite(redLEDPin, HIGH);
  while (true) {
    if (digitalRead(buttonPin) == LOW) {
      delay(200);
      digitalWrite(buzzerPin, LOW);
      digitalWrite(redLEDPin, LOW);
      lcdPrint("System reset...", "", 2000);
      lcd.clear();
      i = 0;
      state = 0;
      return;
    }
    delay(100);
  }
}

String getFormattedInput(const char* prompt, String placeholder) {
  lcdPrint(prompt, placeholder.c_str());
  int nextIndex = 0;
  while (true) {
    char key = keypad.getKey();
    if (!key) continue;
    if (checkExitKey(key)) return "";
    if (key >= '0' && key <= '9') {
      while (nextIndex < (int)placeholder.length() && placeholder[nextIndex] != 'x') nextIndex++;
      if (nextIndex < (int)placeholder.length()) {
        placeholder[nextIndex++] = key;
        lcd.setCursor(0, 1); 
        lcd.print(placeholder);
      }
    } else if (key == '*') {
      if (nextIndex > 0) {
        nextIndex--;
        while (nextIndex > 0 && placeholder[nextIndex] != 'x' &&
               (placeholder[nextIndex] == '-' || placeholder[nextIndex] == '.')) nextIndex--;
        if (placeholder[nextIndex] != '-' && placeholder[nextIndex] != '.') {
          placeholder[nextIndex] = 'x';
          lcd.setCursor(0, 1); 
          lcd.print(placeholder);
        }
      }
    } else if (key == '#') {
      if (placeholder.indexOf('x') == -1) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Entered:");
        lcd.setCursor(0, 1);
        lcd.print(placeholder);
        Serial.println(String(prompt) + ": " + placeholder);
        delay(1000);
        return placeholder;
      } else {
        lcdPrint("Incomplete", "input", 1500);
        lcdPrint(prompt, placeholder.c_str());
      }
    } else {
      lcdPrint("Invalid input", "", 1500);
      lcdPrint(prompt, placeholder.c_str());
    }
  }
}

int selectFromList(const char* items[], int count) {
  int page = 0;
  lcd.clear();
  lcd.print(items[page]);
  lcd.setCursor(0, 1);
  lcd.print("B=Next A=Prev");
  while (true) {
    char key = keypad.getKey();
    if (!key) continue;
    if (checkExitKey(key)) return -1;
    if (key == 'B') { page = (page + 1) % count; }
    else if (key == 'A') { page = (page - 1 + count) % count; }
    else if (key >= '1' && key <= ('0' + count)) {
      int idx = key - '1';
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Selected:");
      lcd.setCursor(0, 1);
      lcd.print(items[idx]);
      Serial.println("Selected: " + String(items[idx]));
      delay(2000);
      return idx;
    } else {
      lcdPrint("Invalid input", "", 1500);
    }
    lcd.clear();
    lcd.print(items[page]);
    lcd.setCursor(0, 1);
    lcd.print("B=Next A=Prev");
  }
}

void setup() {
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  WiFi.begin(ssid, pass);
  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  pinMode(panicButtonPin, INPUT_PULLUP);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  pinMode(redLEDPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
  digitalWrite(redLEDPin, LOW);
}

void loop() {
  Blynk.run();
  static bool panicTriggered = false;

  if (i == 0) { Serial.println("System initialised..."); i++; }

  // Panic check
  if (digitalRead(panicButtonPin) == LOW && !panicTriggered) {
    delay(50);
    if (digitalRead(panicButtonPin) == LOW) {
      panicTriggered = true;
      panicMode();
      panicTriggered = false;
      return;
    }
  }

  if (state == 0) {
    if (digitalRead(buttonPin) == LOW) {
      delay(100);
      lcdPrint("Welcome to GBV", "Reporting System", 2000);
      state = 1;
    }

  } else if (state == 1) {
    lcdPrint("We are here", "to help you", 2000);
    state = 2;

  } else if (state == 2) {
    String date = getFormattedInput("Enter Date:", "xx-xx-xxxx");
    if (date == "") return;
    state = 3;

  } else if (state == 3) {
    String t = getFormattedInput("Enter Time:", "xx.xx");
    if (t == "") return;
    ThingSpeak.setField(2, t.toFloat());
    state = 4;

  } else if (state == 4) {
    lcdPrint("Select Location", "", 1000);
    int loc = selectFromList(locations, 4);
    if (loc < 0) return;
    ThingSpeak.setField(1, loc);
    state = 5;

  } else if (state == 5) {
    lcdPrint("Please select", "your type of", 1000);
    lcdPrint("violence...", "", 1500);
    int vtype = selectFromList(violenceTypes, 5);
    if (vtype < 0) return;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Violence Type:");
    lcd.setCursor(0, 1);
    lcd.print(violenceTypes[vtype]);
    Serial.println("Violence Type: " + String(violenceTypes[vtype]));

    Blynk.logEvent(blynkEvents[vtype], blynkMsg[vtype]);
    delay(3000);

    int x = ThingSpeak.writeFields(myChannelNumber, myApiKey);
    Serial.println(x == 200 ? "Data pushed successfully!" : "Push error, retrying...");

    // Show response pages from data table
    for (int p = 0; p < 4; p++) {
      if (strlen(responsePages[vtype][p]) == 0) break;
      String page = String(responsePages[vtype][p]);
      int sep = page.indexOf('\n');
      lcdPrint(page.substring(0, sep).c_str(), page.substring(sep + 1).c_str(), 2500);
    }

    delay(1000);
    lcdPrint("System reset...", "", 1500);
    lcd.clear(); 
    i = 0; 
    state = 0;
  }
}
