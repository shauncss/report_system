#define BLYNK_TEMPLATE_ID "TMPL64RJEo8wl"
#define BLYNK_TEMPLATE_NAME "Alerting system"
#define BLYNK_AUTH_TOKEN "E47GXcMGPAAuiJmacc2wrlSUWQW7sF-h"

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
const char* server = "api.thingspeak.com";
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
const int panicButtonPin = 32;
const int buttonPin = 34;
int state = 0;
int i = 0;

// Voilence types declaration
const char* violenceTypes[] = {
    "1.Physical", "2.Sexual",
    "3.Emotion", "4.Digital",
    "5.Stalking/IPV"
  };

// Locations declaration
const char* locations[] = {
  "1.Hospital PP",
  "2.Tmn Free Schl",
  "3.Bus Terminal",
  "4.USM"
};


void setup() {
  // Start Blynk connection
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


void showPage(int p) {
  lcd.clear();
  lcd.setCursor(0, 0);
  
  if (p >= 0 && p < 5) {
    lcd.print(violenceTypes[p]);  // Show one type per page
    lcd.setCursor(0, 1);
    lcd.print("B=Next A=Prev");
  }
}


void panicMode() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("! PANIC ALERT !");
  lcd.setCursor(0, 1);
  lcd.print("Help is OTW!");

  digitalWrite(buzzerPin, HIGH);
  digitalWrite(redLEDPin, HIGH);

  // Stay in this loop until someone manually resets via pushbutton
  while (true) {
    
    // Check if pushbutton (buttonPin) is pressed
    if (digitalRead(buttonPin) == LOW) {
      delay(200); // debounce
      digitalWrite(buzzerPin, LOW);
      digitalWrite(redLEDPin, LOW);

      lcd.clear();
      lcd.print("System reset...");
      delay(2000);
      lcd.clear();
      i = 0;
      state = 0;
      return; // Exit panic mode
    }
    delay(100);
  }
}

bool checkExitKey(char key) {
  if (key == 'D') {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Exiting...");
    delay(1000);
    lcd.clear();
    i = 0;
    state = 0;
    return true; // exit requested
  }
  return false;
}


void loop() {
  Blynk.run();
  static bool panicTriggered = false;

  if (i == 0) {
    Serial.println("--------------------------");
    Serial.println("System initialised...");
    i++;
  }

  if (digitalRead(panicButtonPin) == LOW && !panicTriggered) {
    delay(50); // debounce
    if (digitalRead(panicButtonPin) == LOW) {
      panicTriggered = true;
      panicMode();  // this will wait for reset from buttonPin (34)
      panicTriggered = false;
      return;
    }
  }
  if (state == 0) //waiting for user detection
  {
    if (digitalRead(buttonPin) == LOW) {
      delay(100);  // Debounce
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Welcome to GBV");
      lcd.setCursor(0, 1);
      lcd.print("Reporting System");
      delay(2000);
      state = 1;
    }
  } else if (state == 1) // consolation
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("We are here");
    lcd.setCursor(0, 1);
    lcd.print("to help you");
    delay(2000);
    state = 2; 
  } 
  else if (state == 2) // Date input
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter Date:");
  
    String dateInput = "xx-xx-xxxx"; // Placeholder
    lcd.setCursor(0, 1);
    lcd.print(dateInput);

    int nextIndex = 0; // Tracks which 'x' to replace next

    while (true) {
      char key = keypad.getKey();
      if (key) {
        if (checkExitKey(key)) return;
        // Only accept digits and control keys
        if (key >= '0' && key <= '9') {
          if (nextIndex < dateInput.length()) {
            // Find next 'x' to replace
            while (nextIndex < dateInput.length() && dateInput[nextIndex] != 'x') {
              nextIndex++;  // skip dashes '-'
            }
            if (nextIndex < dateInput.length()) {
              dateInput[nextIndex] = key;
              lcd.setCursor(0, 1);
              lcd.print(dateInput);
              nextIndex++;
            }
          }
        }
        else if (key == '*') {  // Backspace
          // Move back to previous entered digit and replace it with 'x'
          if (nextIndex > 0) {
            nextIndex--;
            while (nextIndex > 0 && dateInput[nextIndex] != 'x' && dateInput[nextIndex] == '-') {
              nextIndex--;
            }
            if (dateInput[nextIndex] != '-') {
              dateInput[nextIndex] = 'x';
              lcd.setCursor(0, 1);
              lcd.print(dateInput);
            }
          }
        }
        else if (key == '#') {  // Submit
          // Check if all 'x' are replaced or handle partial input as you wish
          if (dateInput.indexOf('x') == -1) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Date Entered:");
            lcd.setCursor(0, 1);
            lcd.print(dateInput);
            Serial.println("Date Entered: " + dateInput);
            delay(1000);
            state = 3;
            return;
          } else {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Incomplete");
            lcd.setCursor(0, 1);
            lcd.print("date input");
            delay(1500);
            lcd.setCursor(0, 1);
            lcd.print(dateInput);
          }
        }
        else if (key == 'A' || key == 'B' || key == 'C' || key == 'D') {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Invalid input");
          delay(1500);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Enter Date:");
          lcd.setCursor(0, 1);
          lcd.print(dateInput); 
        }
      }
    }
  }
  else if (state == 3) // Time input
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter Time:");

    String timeInput = "xx.xx"; // Placeholder for time
    lcd.setCursor(0, 1);
    lcd.print(timeInput);

    int nextIndex = 0;

    while (true) {
      char key = keypad.getKey();
      if (key) {
        if (checkExitKey(key)) return;
        if (key >= '0' && key <= '9') {
          while (nextIndex < timeInput.length() && timeInput[nextIndex] != 'x') {
            nextIndex++;  // Skip colon
          }
          if (nextIndex < timeInput.length()) {
            timeInput[nextIndex] = key;
            lcd.setCursor(0, 1);
            lcd.print(timeInput);
            nextIndex++;
          }
        }
        else if (key == '*') { // Backspace
          if (nextIndex > 0) {
            nextIndex--;
            while (nextIndex > 0 && timeInput[nextIndex] == ':') {
              nextIndex--;
            }
            if (timeInput[nextIndex] != '.') {
              timeInput[nextIndex] = 'x';
              lcd.setCursor(0, 1);
              lcd.print(timeInput);
            }
          }
        }
        else if (key == '#') { // Submit
          if (timeInput.indexOf('x') == -1) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Time Entered:");
            lcd.setCursor(0, 1);
            lcd.print(timeInput);
            Serial.println("Time Entered: " + timeInput);
            float timenum = timeInput.toFloat(); // change time to float
            ThingSpeak.setField(2, timenum); //set to thingspeak field 2
            delay(3000); // Show confirmation for 3 seconds
            state = 4;   // Proceed to next state (if any)
            return;
          } else {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Incomplete");
            lcd.setCursor(0, 1);
            lcd.print("time input");
            delay(1500);
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Enter Time:");
            lcd.setCursor(0, 1);
            lcd.print(timeInput);
          }
        }
        else if (key == 'A' || key == 'B' || key == 'C' || key == 'D') {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Invalid input");
          delay(1500);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Enter Time:");
          lcd.setCursor(0, 1);
          lcd.print(timeInput);
        }
      }
    }
  }
  else if (state == 4) { // Location Selection
    int page = 0;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Select Location");
    delay(1000);
    lcd.clear();
    lcd.print(locations[page]);
    lcd.setCursor(0, 1);
    lcd.print("B=Next A=Prev");

    while (true) {
      char key = keypad.getKey();
      if (key) {
        if (checkExitKey(key)) return;
        if (key == 'B') {
          page = (page + 1) % 4;
        } else if (key == 'A') {
          page = (page - 1 + 4) % 4;
        } else if (key >= '1' && key <= '4') {
          int index = key - '1';
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Location:");
          lcd.setCursor(0, 1);
          lcd.print(locations[index]);
          Serial.println("Location: " + String(locations[index]));
          int locanum = index++; //declare location
          ThingSpeak.setField(1, locanum); //set to thingspeak field 1
          delay(3000);
          state = 5;
          return;
        } else {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Invalid input");
          delay(1500);
        }
        // Refresh current page
        lcd.clear();
        lcd.print(locations[page]);
        lcd.setCursor(0, 1);
        lcd.print("B=Next A=Prev");
      }
    }
  }

  else if (state == 5) // Violence type input with paging
  {
    int page = 0;
    char selection = '\0';
    // Show the initial message
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Please select");
    lcd.setCursor(0, 1);
    lcd.print("your type of");
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("violence...");
    delay(1500);
    showPage(page);

    while (true) {
      char key = keypad.getKey();
      if (key) {
        if (checkExitKey(key)) return;
        if (key == 'B') {
          page = (page + 1) % 5;
          showPage(page);
        }else if (key == 'A') {  // Previous page
          page = (page - 1 + 5) % 5;  // ensures wraparound
          showPage(page);
        }else if (key >= '1' && key <= '5') {
        selection = key;
        lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Violence Type:");
  lcd.setCursor(0, 1);
  lcd.print(violenceTypes[selection - '1']);
  Serial.println("Violence Type: " + String(violenceTypes[selection - '1']));

  // 🔔 Trigger Blynk Virtual Pins based on type
  switch (selection) {
    case '1': // Physical
      Blynk.logEvent("physical_alert", "Physical violence alert triggered!");
      break;
    case '2': // Sexual
      Blynk.logEvent("sexual_alert", "Sexual violence alert triggered!");
      break;
    case '3': // Emotional
      Blynk.logEvent("emotional_alert", "Emotional violence alert triggered!");
      break;
    case '4': // Digital
      Blynk.logEvent("digital_alert", "Digital violence alert triggered!");
      break;
    case '5': // Stalking/IPV
      Blynk.logEvent("stalking_alert", "Stalking violence alert triggered!");
      break;
  }
  delay(3000);

  int x = ThingSpeak.writeFields(myChannelNumber,myApiKey); // write data to thingspeak
    
  if (x == 200) {
    Serial.println("Data pushed sucessfully !!");
  } else {
    Serial.println("Push error, retrying ... ");
  }

  lcd.clear();
  switch (selection) {
    case '1': // Physical
      lcd.setCursor(0, 0);
      lcd.print("Alert sent to");
      lcd.setCursor(0, 1);
      lcd.print("local emergency");
      delay(2000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("services.");
      delay(1500);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("You will");
      lcd.setCursor(0, 1);
      lcd.print("be safe!");
      break;

    case '2': // Sexual
      lcd.setCursor(0, 0);
      lcd.print("Alert sent to");
      lcd.setCursor(0, 1);
      lcd.print("KPWKM. Stay safe");
      delay(1500);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("You're not alone.");
      lcd.setCursor(0, 1);
      lcd.print("Help is coming.");
      break;

    case '3': // Emotional
      lcd.setCursor(0, 0);
      lcd.print("Call WAO now:");
      lcd.setCursor(0, 1);
      lcd.print("03 3000 8858");
      delay(4000);
      lcd.clear();
      lcd.print("You're not alone.");
      lcd.setCursor(0, 1);
      lcd.print("Help is here.");
      break;

    case '4': // Digital
      lcd.setCursor(0, 0);
      lcd.print("Alert sent to");
      lcd.setCursor(0, 1);
      lcd.print("CCID");
      delay(1500);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Don't worry!");
      delay(1500);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Tips: Change");
      lcd.setCursor(0, 1);
      lcd.print("passwords often");
      delay(3000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Enable 2-Factor");
      lcd.setCursor(0, 1);
      lcd.print("Authentication");
      delay(3000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("& avoid");
      lcd.setCursor(0, 1);
      lcd.print("suspicious links");
      delay(1500);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(" or emails.");
      break;

    case '5': // Stalking/IPV
      lcd.setCursor(0, 0);
      lcd.print("Dial 999 or call");
      lcd.setCursor(0, 1);
      lcd.print("trusted contact");
      delay(3000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Need shelter?");
      lcd.setCursor(0, 1);
      lcd.print("Call 03-7770 9999");
      delay(4000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Stay calm.");
      lcd.setCursor(0, 1);
      lcd.print("You're not alone.");
      break;
  }
      delay(4000);  // Pause before moving to next state
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("System reset...");
      delay(1500);
      lcd.clear();
      i = 0;
      state = 0;
      return;
  }else {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Invalid input");
          delay(1500);
          showPage(page);
        }
      }
    }
  }
}
