// =============================================================
//  Motor.ino  --  L298N dual-H-bridge motor driver
//  Tab 2 of 7
//
//  The L298N drives two DC motors:
//      IN1 + IN2  -> Motor A  (left  wheel)
//      IN3 + IN4  -> Motor B  (right wheel)
//
//  Direction truth table (ENA / ENB jumpers are ON the board):
//
//      IN1 IN2  |  Motor A (left)
//     ----------+----------------
//       0   0   |  Coast (no drive)
//       1   0   |  Forward
//       0   1   |  Reverse
//       1   1   |  Brake
//
//  Same pattern for IN3 / IN4 (right wheel).
//
//  All five motion helpers below are simple, with no delays
//  inside -- callers can layer timing on top if they need it.
// =============================================================


// Configure the four motor pins as outputs and stop the car.
void motorSetup() {
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  pinMode(IN3_PIN, OUTPUT);
  pinMode(IN4_PIN, OUTPUT);
  motorStop();
}

// Coast to a stop (no active braking).
void motorStop() {
  digitalWrite(IN1_PIN, LOW);
  digitalWrite(IN2_PIN, LOW);
  digitalWrite(IN3_PIN, LOW);
  digitalWrite(IN4_PIN, LOW);
  isMovingForward = false;
}

// True while the car is currently driving forward.
// Set by motorForward(), cleared by every other motion helper.
// Used by the drift-bias logic in Main.ino's loop() to apply a
// periodic nudge to one wheel when DRIFT_BIAS_MS is non-zero.
bool isMovingForward = false;

// Both wheels spin forward.
void motorForward() {
  digitalWrite(IN1_PIN, HIGH);  // Left  forward
  digitalWrite(IN2_PIN, LOW);
  digitalWrite(IN3_PIN, HIGH);  // Right forward
  digitalWrite(IN4_PIN, LOW);
  isMovingForward = true;
}

// Both wheels spin in reverse.
void motorBackward() {
  digitalWrite(IN1_PIN, LOW);   // Left  reverse
  digitalWrite(IN2_PIN, HIGH);
  digitalWrite(IN3_PIN, LOW);   // Right reverse
  digitalWrite(IN4_PIN, HIGH);
  isMovingForward = false;
}

// Pivot left: left wheel in reverse, right wheel forward.
void motorLeft() {
  digitalWrite(IN1_PIN, LOW);
  digitalWrite(IN2_PIN, HIGH);
  digitalWrite(IN3_PIN, HIGH);
  digitalWrite(IN4_PIN, LOW);
  isMovingForward = false;
}

// Pivot right: left wheel forward, right wheel in reverse.
void motorRight() {
  digitalWrite(IN1_PIN, HIGH);
  digitalWrite(IN2_PIN, LOW);
  digitalWrite(IN3_PIN, LOW);
  digitalWrite(IN4_PIN, HIGH);
  isMovingForward = false;
}
