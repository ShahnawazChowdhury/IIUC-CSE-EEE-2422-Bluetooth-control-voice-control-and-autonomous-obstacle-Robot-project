// =============================================================
//  Sensors.ino  --  Ultrasonic + IR cliff sensor helpers
//  Tab 3 of 7
//
//  Two sensors are used by the autonomous mode:
//
//      1. HC-SR04 ultrasonic  -- measures how far the nearest
//         object in front of the car is, in centimetres.
//
//      2. IR cliff sensor     -- a digital reflective sensor
//         pointed down at the floor.  When it sees the floor
//         its output is LOW.  When there is no floor (i.e. a
//         cliff or table edge), its output goes HIGH.
// =============================================================


// Configure both sensors.
void sensorsSetup() {
  // Ultrasonic
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);

  // IR cliff
  pinMode(IR_PIN, INPUT);
}


// Trigger an ultrasonic pulse and return the measured
// distance in centimetres, or -1 if the echo timed out.
long readUltrasonicCM() {
  // 1. Start with the trigger line LOW.
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  // 2. Send a 10 us HIGH pulse to start a measurement.
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // 3. Measure how long the ECHO pin stays HIGH.
  //    Speed of sound: ~0.0343 cm/us, divide by 2 for round trip.
  long duration = pulseIn(ECHO_PIN, HIGH, ULTRA_TIMEOUT_US);
  if (duration == 0) return -1;          // no echo within timeout
  return (long)(duration * 0.0343 / 2.0);
}


// Returns true when the IR sensor no longer sees the floor
// underneath the car (a cliff / table edge).
bool cliffDetected() {
  return digitalRead(IR_PIN) == HIGH;
}
