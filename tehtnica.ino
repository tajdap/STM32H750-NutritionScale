#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "HX711.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// PINS
const int PIN_BTN_MULTI = 5;
const int PIN_BTN_NEXT = 23;
const int PIN_BTN_PREV = 22;
const int PIN_SCK_DISP = 21;
const int PIN_SDA_DISP = 19;

const int PIN_HX_DOUT = 4;
const int PIN_HX_SCK = 15;
float calibrationFact = -448.06;
HX711 scale;

// FOOD MENU
struct Food {
  String food;
  float calories;
  float protein;
  float fat;
  float carbs;
};

Food foodMenu[] = {
  {"Mleta govedina", 217, 26, 10, 0},
  {"Jajca", 143, 12.5, 10, 1.1},
  {"Riz", 350, 2.7, 1.1, 28},
  {"Testenine", 350, 13, 1.5, 25},
  {"Smetana za kuh.", 200, 2.5, 27, 2.8},
  {"Polnozrnati kruh", 247, 13, 3.4, 41},
  {"Piscancje prsi", 120, 22, 3.6, 0},
  {"Grski jogurt", 59, 10, 0.4, 4},
  {"Pelati", 64, 1.6, 2.6, 7.6},
  {"Kraski prsut", 252, 30, 14, 0}
};

// VALUES
float caloriesFinal = 0;
float proteinFinal = 0;
float fatFinal = 0;
float carbsFinal = 0;
int foodIdx = 0;

// BUTTON PRESS
unsigned long pressedTime = 0;
bool wasPressed = false;

void refreshDisplayMenu() {
  float weight = scale.get_units(5);
  if (weight < 0) {
    weight = 0;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0,0);
  display.println("--- IZBIRA HRANE ---");

  display.setCursor(0, 22);
  display.write(27);
  display.print(" ");
  display.print(foodMenu[foodIdx].food);
  display.print(" ");
  display.write(26);
  display.println();

  display.setCursor(0, 42);
  display.setTextSize(2);
  display.print(weight, 0); display.print(" g");

  display.display();
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_BTN_NEXT, INPUT_PULLUP);
  pinMode(PIN_BTN_PREV, INPUT_PULLUP);
  pinMode(PIN_BTN_MULTI, INPUT_PULLUP);

  Wire.begin(PIN_SDA_DISP, PIN_SCK_DISP);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println("SSD1306 allocation failed");
    for(;;);
  }

  scale.begin(PIN_HX_DOUT, PIN_HX_SCK);
  scale.set_scale(calibrationFact);
  scale.tare();

  refreshDisplayMenu();
}

void loop() {
  static unsigned long lastPulse = 0;
    if (millis() - lastPulse > 300) {
      refreshDisplayMenu();
     lastPulse = millis();
    }

  if (digitalRead(PIN_BTN_NEXT) == LOW) {
    foodIdx++;
    if (foodIdx >= 10) foodIdx = 0;
    refreshDisplayMenu();
    delay(250);
  }
  if (digitalRead(PIN_BTN_PREV) == LOW) {
    foodIdx--;
    if (foodIdx < 0) foodIdx = 9;
    refreshDisplayMenu();
    delay(250);
  }

  // Multifunction button
  if (digitalRead(PIN_BTN_MULTI) == LOW) {
    if (wasPressed == false) {
      pressedTime = millis();
      wasPressed = true;
    }

    if (millis() - pressedTime > 1000) {
      display.clearDisplay();
      display.setCursor(0, 25);
      display.setTextSize(1);
      display.println("Zaključujem...");
      display.display();
    }
  } else {
    if (wasPressed == true) {
      unsigned long pressDuration = millis() - pressedTime;
      wasPressed = false;

      if (pressDuration > 1000) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.println("--- SKUPNI OBROK ---");
        
        display.setCursor(0, 16);
        display.print("Kcal: "); display.print(caloriesFinal, 1); display.println(" kcal");
        display.print("B:    "); display.print(proteinFinal, 1); display.println(" g");
        display.print("M:    "); display.print(fatFinal, 1); display.println(" g");
        display.print("OH:   "); display.print(carbsFinal, 1); display.println(" g");
        display.display();

        Serial.println("\n--- SKUPNI OBROK ---");
        Serial.print("Kalorije: "); Serial.println(caloriesFinal);
        
        delay(5000); 
        
        caloriesFinal = 0; proteinFinal = 0; fatFinal = 0; carbsFinal = 0;
        refreshDisplayMenu();
      } else {
        Food selected = foodMenu[foodIdx];
        float weight = scale.get_units(10);
        if (weight < 0) {
          weight = 0;
        }

        caloriesFinal += (weight * selected.calories) / 100.0;
        proteinFinal += (weight * selected.protein) / 100.0;
        fatFinal += (weight * selected.fat) / 100.0;
        carbsFinal += (weight * selected.carbs) / 100.0;

        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0, 20);
        display.println("DODANO K OBROKU!");
        display.setCursor(0, 40);
        display.print("+ "); display.print(weight, 0); display.print("g "); display.println(selected.food);
        display.display();

        delay(1000);
        refreshDisplayMenu();
          }
    }
  }

}
