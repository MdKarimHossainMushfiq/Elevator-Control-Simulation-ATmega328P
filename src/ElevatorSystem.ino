#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const int pin_in[4] = {2, 3, 4, 5};       // Inside buttons (G, 1, 2, 3)
const int pin_up[4] = {A0, A1, A2, -1};   // UP buttons (-1 means no UP button on Top Floor)
const int pin_dn[4] = {-1, 13, 0, 1};     // DOWN buttons (-1 means no DOWN button on Ground Floor)

const int ledPins[4]  = {6, 7, 8, 9};     // Floor LEDs 

const int motorUpPin = 10;                // L293D IN1 / UP
const int motorDownPin = 11;              // L293D IN2 / DOWN
const int doorSensorPin = 12;             // IR Sensor (LOGICSTATE in Proteus)

// DISPLAY SETUP
// Address is 0x20 because A0, A1, A2 on the PCF8574 are wired to Ground
LiquidCrystal_I2C lcd(0x20, 16, 2); 

// REQUEST MEMORY 
bool req_in[4] = {0}; 
bool req_up[4] = {0}; 
bool req_dn[4] = {0}; 

int currentFloor = 1; 
#define IDLE 0 
#define UP 1 
#define DOWN 2 
int dir = IDLE; 


void setup() { 
  // Initialize floor buttons and LEDs
  for (int i = 0; i < 4; i++) { 
    pinMode(pin_in[i], INPUT_PULLUP); 
    if (pin_up[i] != -1) pinMode(pin_up[i], INPUT_PULLUP); // Only init if pin exists
    if (pin_dn[i] != -1) pinMode(pin_dn[i], INPUT_PULLUP); // Only init if pin exists
    pinMode(ledPins[i], OUTPUT); 
  } 
  
  pinMode(motorUpPin, OUTPUT); 
  pinMode(motorDownPin, OUTPUT); 
  pinMode(doorSensorPin, INPUT_PULLUP); // Initialize safety sensor

  // Initialize the LCD
  lcd.init();
  lcd.backlight();
  stopMotor(); 
  updateDisplaysAndLEDs(); 
} 

// DISPLAY & LED LOGIC 
void updateDisplaysAndLEDs() { 
  // Update the original individual LEDs
  for (int i = 0; i < 4; i++) {
    digitalWrite(ledPins[i], LOW); 
  }
  digitalWrite(ledPins[currentFloor - 1], HIGH); 

  // Update the LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Floor: ");

  // Print Floor Number/Letter
  if (currentFloor == 1) lcd.print("G");
  else if (currentFloor == 2) lcd.print("1");
  else if (currentFloor == 3) lcd.print("2");
  else if (currentFloor == 4) lcd.print("3");

  // Print Direction Status on the second line
  lcd.setCursor(0, 1);
  if (dir == UP) {
    lcd.print("Status: UP      ");
  } 
  else if (dir == DOWN) {
    lcd.print("Status: DOWN    ");
  }
  else {
    lcd.print("Status: IDLE    ");
  }
}

// BUTTON READ 
void readButtons() { 
  for (int i = 0; i < 4; i++) { 
    if (digitalRead(pin_in[i]) == LOW) req_in[i] = true; 
    
    // Only read external buttons if they actually exist (!= -1)
    if (pin_up[i] != -1 && digitalRead(pin_up[i]) == LOW) req_up[i] = true; 
    if (pin_dn[i] != -1 && digitalRead(pin_dn[i]) == LOW) req_dn[i] = true; 
  } 
} 

// Smart Delay: Allows code to pause but still catch button presses
void smartDelay(int ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    readButtons();
    delay(5); 
  }
}

// CHECK REQUESTS 
bool hasAbove(int f) { 
  for (int i = f; i < 4; i++) { 
    if (req_in[i] || req_up[i] || req_dn[i]) return true; 
  } 
  return false; 
} 

bool hasBelow(int f) { 
  for (int i = f - 2; i >= 0; i--) { 
    if (req_in[i] || req_up[i] || req_dn[i]) return true; 
  } 
  return false; 
} 

// MOTOR 
void stopMotor() { 
  digitalWrite(motorUpPin, LOW); 
  digitalWrite(motorDownPin, LOW); 
} 

void moveUp() { 
  digitalWrite(motorUpPin, HIGH); 
  digitalWrite(motorDownPin, LOW); 
} 

void moveDown() { 
  digitalWrite(motorUpPin, LOW); 
  digitalWrite(motorDownPin, HIGH); 
} 

// DOOR & SAFETY LOGIC 
void openDoor() { 
  stopMotor(); 
  
  // Display door opening message
  lcd.setCursor(0, 1);
  lcd.print(" **DOORS OPEN** "); 
  smartDelay(1500); // Wait 1.5 seconds for passengers
  
  // SAFETY SENSOR CHECK
  // If the IR sensor pin is LOW (LOGICSTATE = 0), someone is in the way.
  while (digitalRead(doorSensorPin) == LOW) {
    lcd.setCursor(0, 1);
    lcd.print(" DOOR BLOCKED!  "); 
    delay(100); 
    // We don't use smartDelay here so the lift strictly waits for clearance
  }

  // Display door closing message once path is clear
  lcd.setCursor(0, 1);
  lcd.print(" *DOORS CLOSING*"); 
  smartDelay(1000); // Wait 1 second for doors to close

  // Restore the normal floor/status screen
  updateDisplaysAndLEDs(); 
} 

// MAIN LOOP 
void loop() { 
  readButtons(); 

  // IDLE LOGIC 
  if (dir == IDLE) { 
    if (hasAbove(currentFloor)) {
      dir = UP;
      updateDisplaysAndLEDs(); 
    } 
    else if (hasBelow(currentFloor)) {
      dir = DOWN;
      updateDisplaysAndLEDs(); 
    } 
    else if (req_in[currentFloor-1] || req_up[currentFloor-1] || req_dn[currentFloor-1]) {
       openDoor();
       req_in[currentFloor-1] = false;
       req_up[currentFloor-1] = false;
       req_dn[currentFloor-1] = false;
    }
  } 

  // UP LOGIC 
  if (dir == UP) { 
    moveUp(); 
    smartDelay(1000); 
    currentFloor++; 
    updateDisplaysAndLEDs(); 

    bool stop = false; 
    if (req_in[currentFloor - 1] || req_up[currentFloor - 1]) stop = true; 
    if (!hasAbove(currentFloor) && req_dn[currentFloor - 1]) stop = true; 

    if (stop) { 
      dir = IDLE; 
      openDoor(); 
      dir = UP; 
      
      req_in[currentFloor - 1] = false; 
      req_up[currentFloor - 1] = false; 
      req_dn[currentFloor - 1] = false; 
    } 

    if (!hasAbove(currentFloor)) { 
      if (hasBelow(currentFloor)) dir = DOWN; 
      else dir = IDLE; 
      updateDisplaysAndLEDs();
    } 
  } 

  // DOWN LOGIC 
  else if (dir == DOWN) { 
    moveDown(); 
    smartDelay(1000); 
    currentFloor--; 
    updateDisplaysAndLEDs(); 

    bool stop = false; 
    if (req_in[currentFloor - 1] || req_dn[currentFloor - 1]) stop = true; 
    if (!hasBelow(currentFloor) && req_up[currentFloor - 1]) stop = true; 

    if (stop) { 
      dir = IDLE; 
      openDoor(); 
      dir = DOWN; 
      
      req_in[currentFloor - 1] = false; 
      req_dn[currentFloor - 1] = false; 
      req_up[currentFloor - 1] = false; 
    } 

    if (!hasBelow(currentFloor)) { 
      if (hasAbove(currentFloor)) dir = UP; 
      else dir = IDLE; 
      updateDisplaysAndLEDs();
    } 
  } 
  delay(5); 
}
