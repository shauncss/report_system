# GBV Reporting System

This project is an ESP32-based reporting and alerting system designed to assist victims of Gender-Based Violence (GBV). It provides a guided, step-by-step interface to report incidents and features a dedicated panic mode for immediate emergencies.

## Features
* **Panic Alert:** A dedicated push button instantly triggers an active buzzer and a red LED, while displaying a "! PANIC ALERT !" message on the LCD screen.
* **Structured Reporting:** Users can securely input the incident's date, time, location, and the specific type of violence through a 4x4 membrane keypad.
* **Cloud Logging:** The system connects to WiFi to log the incident's time and location data to ThingSpeak for record-keeping.
* **Real-time Alerts:** The system integrates with Blynk to send specific virtual event alerts based on the type of violence reported (e.g., physical, sexual, emotional, digital, or stalking).
* **Resource Guidance:** The LCD provides helpful immediate actions and emergency contacts, such as WAO, KPWKM, CCID, or dialing 999, tailored to the user's selected violence category.

## Hardware Components
* ESP32 Development Board (DevKit V4)
* I2C LCD 1602 Display
* 4x4 Membrane Keypad
* 2x Pushbuttons (Green for navigation/system reset, Red for Panic)
* Active Buzzer
* Red LED
* Resistors (10kΩ and 220Ω)

## Required Libraries
Ensure the following libraries are installed in your Arduino IDE or Wokwi environment:
* `Keypad`
* `LiquidCrystal I2C`
* `WiFi`
* `Blynk` (along with `BlynkESP32_BT_WF`)
* `ThingSpeak`

## Setup Instructions
1. **Hardware Connections:** Wire the components as per the provided layout. The keypad utilizes pins 19, 18, 5, 17 for its rows and 16, 4, 0, 2 for its columns. The buzzer connects to pin 25, the red LED to pin 26, the panic button to pin 32, and the action button to pin 34.
2. **WiFi Setup:** Update the `ssid` and `pass` variables with your network credentials. By default, the code uses `Wokwi-GUEST` with no password for seamless simulation.
3. **Blynk Configuration:** Replace `BLYNK_TEMPLATE_ID`, `BLYNK_TEMPLATE_NAME`, and `BLYNK_AUTH_TOKEN` at the top of the sketch with your own Blynk project details.
4. **ThingSpeak Configuration:** Insert your designated `myChannelNumber` and `myApiKey` to enable data logging.

## How to Use
1. **Initialization:** Press the action button (connected to pin 34) to clear the wait state and initialize the "Welcome to GBV Reporting System" screen.
2. **Input Data:**
    * Use the keypad to enter the Date and Time when prompted.
    * Navigate the menu to select the incident location (Hospital PP, Tmn Free Schl, Bus Terminal, USM).
    * Choose the precise violence type (Physical, Sexual, Emotion, Digital, Stalking/IPV).
3. **Panic Mode:** At any given moment, pressing the red panic button will instantly trigger the visual and audible alarms. To deactivate the alarm, press the primary action button to reset the system.
