#include <SoftwareSerial.h>
#include <Servo.h>

/* ===================== PIN DEFINITIONS ===================== */
// --- Bluetooth ---
#define BT_RX_PIN     /* e.g. 2 */
#define BT_TX_PIN     /* e.g. 3 */

// --- Mode Switch ---
#define MODE_PIN      /* e.g. 8 */

// --- Motor Driver (SN754410) ---
#define ENA_PIN       /* e.g. 10 */
#define IN1_PIN       /* e.g. 4  */
#define IN2_PIN       /* e.g. 5  */
#define IN3_PIN       /* e.g. 6  */
#define IN4_PIN       /* e.g. 7  */
#define ENB_PIN       /* e.g. 9  */

// --- Ultrasonic Sensors ---
#define TRIG_FRONT    /* e.g. A0 */
#define ECHO_FRONT    /* e.g. A1 */
#define TRIG_REAR     /* e.g. A2 */
#define ECHO_REAR     /* e.g. A3 */

// --- Servo ---
#define SERVO_PIN     /* e.g. 11 */

/* ===================== CONSTANTS ===================== */
#define FRONT_MIN_DIST  20   // cm
#define REAR_MIN_DIST   15   // cm

#define SERVO_LEFT      30
#define SERVO_CENTER    90
#define SERVO_RIGHT     150

/* ===================== OBJECTS ===================== */
SoftwareSerial BT(BT_RX_PIN, BT_TX_PIN);
Servo scanServo;

/* ===================== MODES ===================== */
enum Mode { AUTONOMOUS, ARCADE };
Mode currentMode;

/* ===================== GLOBALS ===================== */
int xAxis = 140, yAxis = 140;
int motorSpeedA = 0;
int motorSpeedB = 0;

int frontDist = 200;
int rearDist  = 200;

/* ===================== SETUP ===================== */
void setup() {

  // Motor pins
  pinMode(ENA_PIN, OUTPUT);
  pinMode(ENB_PIN, OUTPUT);
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  pinMode(IN3_PIN, OUTPUT);
  pinMode(IN4_PIN, OUTPUT);

  // Ultrasonic pins
  pinMode(TRIG_FRONT, OUTPUT);
  pinMode(ECHO_FRONT, INPUT);
  pinMode(TRIG_REAR, OUTPUT);
  pinMode(ECHO_REAR, INPUT);

  // Mode switch (INTERNAL PULL-UP)
  pinMode(MODE_PIN, INPUT_PULLUP);

  // Servo
  scanServo.attach(SERVO_PIN);
  scanServo.write(SERVO_CENTER);

  Serial.begin(9600);
  BT.begin(9600);

  stop();
}

/* ===================== MAIN LOOP ===================== */
void loop() {

  // Read mode
  currentMode = (digitalRead(MODE_PIN) == LOW) ? AUTONOMOUS : ARCADE;

  // Update sensor readings
  updateDistances();

  if (currentMode == AUTONOMOUS) {
    autonomousMode();
  } else {
    arcadeMode();
  }

  analogWrite(ENA_PIN, motorSpeedA);
  analogWrite(ENB_PIN, motorSpeedB);
}

/* ===================== MODES ===================== */

void arcadeMode() {

  readBluetoothJoystick();

  // SAFETY OVERRIDES
  if (frontDist < FRONT_MIN_DIST && yAxis < 130) {
    stop();
    motorSpeedA = 0;
    motorSpeedB = 0;
    return;
  }

  if (rearDist < REAR_MIN_DIST && yAxis > 150) {
    stop();
    motorSpeedA = 0;
    motorSpeedB = 0;
    return;
  }

  joystickToMotors();
}

void autonomousMode() {

  if (frontDist > FRONT_MIN_DIST) {
    forward();
    motorSpeedA = 170;
    motorSpeedB = 170;
  } else {
    stop();
    scanAndTurn();
  }
}

/* ===================== BLUETOOTH ===================== */

void readBluetoothJoystick() {
  while (BT.available() >= 2) {
    xAxis = BT.read();
    delay(5);
    yAxis = BT.read();
  }
}

/* ===================== JOYSTICK LOGIC ===================== */

void joystickToMotors() {

  if (xAxis > 130 && xAxis < 150 && yAxis > 130 && yAxis < 150) {
    stop();
    motorSpeedA = 0;
    motorSpeedB = 0;
    return;
  }

  if (yAxis > 130 && yAxis < 150) {

    if (xAxis < 130) {
      turnRight();
      motorSpeedA = map(xAxis, 130, 60, 0, 255);
      motorSpeedB = map(xAxis, 130, 60, 0, 255);
    }

    if (xAxis > 150) {
      turnLeft();
      motorSpeedA = map(xAxis, 150, 220, 0, 255);
      motorSpeedB = map(xAxis, 150, 220, 0, 255);
    }

  } else {

    if (xAxis > 130 && xAxis < 150) {

      if (yAxis < 130) forward();
      if (yAxis > 150) backward();

      motorSpeedA = map(abs(yAxis - 140), 0, 80, 0, 255);
      motorSpeedB = motorSpeedA;

    } else {

      if (yAxis < 130) forward();
      if (yAxis > 150) backward();

      if (xAxis < 130) {
        motorSpeedA = map(xAxis, 130, 60, 255, 50);
        motorSpeedB = 255;
      }

      if (xAxis > 150) {
        motorSpeedA = 255;
        motorSpeedB = map(xAxis, 150, 220, 255, 50);
      }
    }
  }
}

/* ===================== ULTRASONIC ===================== */

int readUltrasonic(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);
  if (duration == 0) return 300;

  return duration * 0.034 / 2;
}

void updateDistances() {
  frontDist = readUltrasonic(TRIG_FRONT, ECHO_FRONT);
  rearDist  = readUltrasonic(TRIG_REAR,  ECHO_REAR);
}

/* ===================== AUTONOMOUS SCAN ===================== */

void scanAndTurn() {

  int leftDist, rightDist;

  scanServo.write(SERVO_LEFT);
  delay(200);
  leftDist = readUltrasonic(TRIG_FRONT, ECHO_FRONT);

  scanServo.write(SERVO_RIGHT);
  delay(200);
  rightDist = readUltrasonic(TRIG_FRONT, ECHO_FRONT);

  scanServo.write(SERVO_CENTER);

  if (leftDist > rightDist) {
    turnLeft();
  } else {
    turnRight();
  }

  motorSpeedA = 150;
  motorSpeedB = 150;
  delay(300);
}

/* ===================== MOTOR CONTROL ===================== */

void forward() {
  digitalWrite(IN1_PIN, HIGH);
  digitalWrite(IN2_PIN, LOW);
  digitalWrite(IN3_PIN, HIGH);
  digitalWrite(IN4_PIN, LOW);
}

void backward() {
  digitalWrite(IN1_PIN, LOW);
  digitalWrite(IN2_PIN, HIGH);
  digitalWrite(IN3_PIN, LOW);
  digitalWrite(IN4_PIN, HIGH);
}

void turnLeft() {
  digitalWrite(IN1_PIN, LOW);
  digitalWrite(IN2_PIN, HIGH);
  digitalWrite(IN3_PIN, HIGH);
  digitalWrite(IN4_PIN, LOW);
}

void turnRight() {
  digitalWrite(IN1_PIN, HIGH);
  digitalWrite(IN2_PIN, LOW);
  digitalWrite(IN3_PIN, LOW);
  digitalWrite(IN4_PIN, HIGH);
}

void stop() {
  digitalWrite(IN1_PIN, LOW);
  digitalWrite(IN2_PIN, LOW);
  digitalWrite(IN3_PIN, LOW);
  digitalWrite(IN4_PIN, LOW);
}
