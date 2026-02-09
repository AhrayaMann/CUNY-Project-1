// --- Ultrasonic Sensor Variables --- 
const int triggerPin = 9; 
const int echoPin = 10; 
float duration, distanceCM, offset = 0, modifiedZ; 

// --- Motor/Output Pins (Fixed Names) --- 
const int ENABLE_PIN_1 = 3; // PWM (Connect to ENA on H-Bridge)
const int INPUT_PIN_1  = 4; // Digital (Connect to IN1 on H-Bridge)
const int INPUT_PIN_2  = 5; // Digital (Connect to IN2 on H-Bridge)

// --- 3D Spatial Array --- 
byte space[4][4][4]; 

// --- Motor Mode Variable --- 
int motorSpeed = 0; 

void setup() { 
  Serial.begin(9600); 
  pinMode(triggerPin, OUTPUT); 
  pinMode(echoPin, INPUT); 
  
  // Initialize Motor Pins
  pinMode(ENABLE_PIN_1, OUTPUT); 
  pinMode(INPUT_PIN_1, OUTPUT); 
  pinMode(INPUT_PIN_2, OUTPUT); 

  memset(space, 0, sizeof(space)); 

  // 1. Get Offset 
  Serial.println("Input the offset in centimeters:"); 
  while (Serial.available() == 0) {} 
  offset = Serial.parseFloat(); 

  // 2. Get Motor Mode (0-5) 
  Serial.println("Select Motor Mode (0-5):"); 
  Serial.println("0=Off, 1=51, 2=102, 3=153, 4=204, 5=255"); 
  while (Serial.available() == 0) {} 
  int mode = Serial.parseInt(); 
  mode = constrain(mode, 0, 5); 

  // Calculate Speed: Mode * 51 
  motorSpeed = mode * 51; 

  Serial.print("Baseline: "); Serial.print(offset); Serial.println(" cm."); 
  Serial.print("Motor Mode: "); Serial.print(mode); 
  Serial.print(" (Speed: "); Serial.print(motorSpeed); Serial.println(")"); 
} 

void loop() { 
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
    // To move the motor, IN1 and IN2 must be DIFFERENT
    digitalWrite(INPUT_PIN_1, HIGH); 
    digitalWrite(INPUT_PIN_2, LOW); 
    
    // Control speed via the Enable Pin
    analogWrite(ENABLE_PIN_1, motorSpeed); 

    // Store the value in the array center [2][2]
    space[zIndex][2][2] = (byte)modifiedZ; 
  } else { 
    // Stop the motor
    analogWrite(ENABLE_PIN_1, 0); 
    digitalWrite(INPUT_PIN_1, LOW); 
    digitalWrite(INPUT_PIN_2, LOW); 
  } 

  // 4. Print Data
  Serial.print("Dist: "); Serial.print(distanceCM);
  Serial.print(" | ModZ: "); Serial.println(modifiedZ); 
  
  delay(500); 
}
