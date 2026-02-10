#include <SoftwareSerial.h>

// --- Bluetooth Setup ---
// Pin 11 = RX, Pin 6 = TX
SoftwareSerial myBT(11, 6); 

// --- Ultrasonic Sensor Variables ---
const int triggerPin = 9;
const int echoPin = 10; 
float duration, distanceCM, offset = 0, modifiedZ;

// --- Motor/Output Pins ---
const int ENABLE_PIN_1 = 3; 
const int INPUT_PIN_1 = 4;  
const int INPUT_PIN_2 = 5;  

// --- 3D Spatial Array ---
byte space[4][4][4]; 

// --- Motor Mode Variable ---
int motorSpeed = 0;

void setup() {
  Serial.begin(9600);
  myBT.begin(9600); 
  
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ENABLE_PIN_1, OUTPUT);
  pinMode(INPUT_PIN_1, OUTPUT);
  pinMode(INPUT_PIN_2, OUTPUT);
  
  memset(space, 0, sizeof(space));

  // Instructions sent to both Monitor and Phone
  Serial.println("System Online.");
  myBT.println("--- Connection Established ---");
  myBT.println("Send Offset (e.g. 25.0) or Mode (0-5)");
}

void loop() {
  // --- Dual Input Handling (Serial & Bluetooth) ---
  if (Serial.available() > 0 || myBT.available() > 0) {
    float inputVal = (Serial.available() > 0) ? Serial.parseFloat() : myBT.parseFloat();
    
    if (inputVal > 5) {
      offset = inputVal;
      // Report back to Phone
      myBT.print(">>> New Offset: "); myBT.print(offset); myBT.println(" cm");
    } else {
      int mode = constrain((int)inputVal, 0, 5);
      motorSpeed = mode * 51;
      // Report back to Phone
      myBT.print(">>> Motor Mode: "); myBT.print(mode); 
      myBT.print(" (Speed: "); myBT.print(motorSpeed); myBT.println(")");
    }
  }

  // --- Ultrasonic Logic ---
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distanceCM = (duration * 0.0343) / 2;
  modifiedZ = offset - distanceCM;

  // --- Array & Motor Logic ---
  int zIndex = map((int)modifiedZ, 0, (int)offset, 0, 3);
  zIndex = constrain(zIndex, 0, 3);

  if (modifiedZ > 0) {
    digitalWrite(INPUT_PIN_1, HIGH);
    digitalWrite(INPUT_PIN_2, LOW);
    analogWrite(ENABLE_PIN_1, motorSpeed);
    space[zIndex][2][2] = (byte)modifiedZ;
  } else {
    analogWrite(ENABLE_PIN_1, 0);
    digitalWrite(INPUT_PIN_1, LOW);
    digitalWrite(INPUT_PIN_2, LOW);
  }

  // --- Send Monitor Info to Bluetooth ---
  // We use a timer so it doesn't flood your phone screen too fast
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 500) {
    myBT.print("Dist: "); myBT.print(distanceCM);
    myBT.print("cm | ModZ: "); myBT.println(modifiedZ);
    
    // Also keep it on the Serial Monitor for debugging
    Serial.print("Dist: "); Serial.print(distanceCM);
    Serial.print(" | ModZ: "); Serial.println(modifiedZ);
    
    lastPrint = millis();
  }
}

