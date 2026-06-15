// =============================================================
//  Autonomous.ino  --  Self-driving behaviour
//  Tab 6 of 7
//
//  When currentMode == MODE_AUTO this file decides what the
//  car should do next, based on what the sensors see.
//
//  Priority order (highest first):
//
//      1. CLIFF
//         The IR sensor no longer sees the floor.
//            -> Stop
//            -> Back up for 500 ms
//            -> Stop
//
//      2. OBSTACLE
//         The ultrasonic measures something closer than
//         OBSTACLE_CM (20 cm).
//            -> Stop
//            -> Back up for 300 ms
//            -> Turn right for 400 ms
//
//      3. CLEAR PATH
//            -> Drive forward
//
//  A small settle delay after each sub-action lets the
//  mechanical / sensor state catch up before the next read.
// =============================================================


// Run a motor action for `ms` milliseconds, then stop.
void runFor(void (*action)(), uint32_t ms) {
  action();          // start the movement
  delay(ms);         // hold it for the requested time
  motorStop();       // stop
  delay(50);         // settle
}


void autonomousLoop() {
  // ---- 1. Cliff check (highest priority) ----
  if (cliffDetected()) {
    Serial.println(F("AUTO: cliff ahead"));
    motorStop();
    runFor(motorBackward, CLIFF_BACK_MS);
    return;
  }

  // ---- 2. Ultrasonic obstacle check ----
  long distance = readUltrasonicCM();
  if (distance > 0 && distance < OBSTACLE_CM) {
    Serial.print(F("AUTO: obstacle at "));
    Serial.print(distance);
    Serial.println(F(" cm"));
    motorStop();
    runFor(motorBackward, OBSTACLE_BACK_MS);
    runFor(motorRight,    OBSTACLE_TURN_MS);
    return;
  }

  // ---- 3. Clear path: just keep moving forward ----
  motorForward();
  delay(60);   // small pause so we don't spam the ultrasonic
}
