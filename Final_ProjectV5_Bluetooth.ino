#include <SoftwareSerial.h>

// Bluetooth Setup ---
// Pin 11 MUST be RX because Pin 6 does not support RX interrupts on Uno/Nano
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
  myBT.begin(9600); // Default baud for HC-05/06
  
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ENABLE_PIN_1, OUTPUT);
  pinMode(INPUT_PIN_1, OUTPUT);
  pinMode(INPUT_PIN_2, OUTPUT);
  
  memset(space, 0, sizeof(space));

  Serial.println("System Ready. Send Offset or Mode (0-5) via Bluetooth/Serial.");
  myBT.println("Bluetooth Link Established.");
}

void loop() {
  // Check for incoming data from Serial OR Bluetooth
  if (Serial.available() > 0 || myBT.available() > 0) {
    float inputVal = (Serial.available() > 0) ? Serial.parseFloat() : myBT.parseFloat();
    
    // Logic: Inputs 0-5 are Motor Modes, >5 is treated as Offset
    if (inputVal > 5) {
      offset = inputVal;
      myBT.print("New Offset: "); myBT.println(offset);
    } else {
      int mode = constrain((int)inputVal, 0, 5);
      motorSpeed = mode * 51;
      myBT.print("Motor Speed Set to: "); myBT.println(motorSpeed);
    }
  }

  // 1. Get raw distance
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distanceCM = (duration * 0.0343) / 2;
  modifiedZ = offset - distanceCM;

  // 2. Update Array Logic
  int zIndex = map((int)modifiedZ, 0, (int)offset, 0, 3);
  zIndex = constrain(zIndex, 0, 3);

  // 3. Apply Motor Logic
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

  // 4. Print Data to Bluetooth
  myBT.print("Dist: "); myBT.print(distanceCM);
  myBT.print(" | ModZ: "); myBT.println(modifiedZ);

  delay(500);
}
