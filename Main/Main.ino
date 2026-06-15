// =============================================================
//  Main.ino  --  ESP8266 Multi-Mode Smart Robot Car
//  Tab 1 of 7
//
//  This sketch runs a NodeMCU-powered 2-wheel robot car with
//  two operating modes:
//
//      MODE_MANUAL  : default at boot.  Accepts commands from
//                     the HC-05 Bluetooth module -- either a
//                     single-character command (F B L R S) from
//                     a car-control app, OR a spoken word
//                     ("go ahead", "backward", "back", "left",
//                     "right", "stop") from a voice-control app.
//                     Both styles are accepted at the same time,
//                     so the user can switch between apps freely.
//
//      MODE_AUTO    : the car drives itself.  It uses the front
//                     IR sensor to detect cliffs and the
//                     HC-SR04 ultrasonic to detect obstacles.
//
//  The push button on D3 toggles between the two modes.
//
//  HARDWARE WIRING (NodeMCU pin label -> what it connects to):
//
//      D1  -> L298N IN1
//      D2  -> L298N IN2
//      D7  -> L298N IN3
//      D8  -> L298N IN4
//           (ENA and ENB jumpers stay ON the L298N board)
//
//      D0  -> HC-SR04 TRIG
//      D6  -> HC-SR04 ECHO  (1k -> D6, 2k -> GND voltage divider)
//
//      D5  -> IR cliff sensor OUT  (HIGH = cliff, LOW = floor)
//
//      D3  -> Mode button  (other leg to GND, uses INPUT_PULLUP)
//
//      HC-05 TX -> NodeMCU RX
//      HC-05 RX -> NodeMCU TX
//      (disconnect BOTH during USB firmware upload)
//
//  HOW THE TABS WORK:
//  The Arduino IDE concatenates every .ino in this folder into
//  one big translation unit (alphabetically).  That means any
//  function or constant declared in one tab is automatically
//  visible to all the others -- no #include needed.
// =============================================================


// -------------------------------------------------------------
//  1.  Mode IDs
// -------------------------------------------------------------
//  Keep this list small.  Two modes are enough for this build.
#define MODE_MANUAL  0   // BT single-char + voice words (default)
#define MODE_AUTO    1   // Autonomous obstacle + cliff avoidance


// -------------------------------------------------------------
//  2.  Pin numbers  (NodeMCU silkscreen names)
// -------------------------------------------------------------
//  Motor driver
//    ENA / ENB jumpers stay ON the L298N board (full speed).
#define IN1_PIN   D1   // Left  motor,  forward
#define IN2_PIN   D2   // Left  motor,  reverse
#define IN3_PIN   D7   // Right motor,  forward
#define IN4_PIN   D8   // Right motor,  reverse

//  Ultrasonic sensor
#define TRIG_PIN  D0   // Trigger pulse out
#define ECHO_PIN  D6   // Echo in (5 V -> 3.3 V via 1k/2k divider)

//  IR cliff sensor
#define IR_PIN    D5   // HIGH = cliff,  LOW = floor

//  Mode button
#define BTN_PIN   D3   // INPUT_PULLUP,  pressed == LOW


// -------------------------------------------------------------
//  3.  Tunable constants
// -------------------------------------------------------------
//  Autonomous behaviour
#define OBSTACLE_CM         20   // Stop + avoid if closer than this
#define ULTRA_TIMEOUT_US    25000 // Echo timeout (about 4.3 m)
#define CLIFF_BACK_MS       500   // How long to back up after a cliff
#define OBSTACLE_BACK_MS    300   // How long to back up after a hit
#define OBSTACLE_TURN_MS    400   // How long to turn right after a hit

//  Button debounce
#define BUTTON_DEBOUNCE_MS  35    // Press must be stable this long

//  Bluetooth parser is in Bluetooth.ino.

//  Forward-drift compensation.  Cheap DC gear motors almost
//  never spin at the same RPM, so a perfectly-symmetric
//  "forward" command usually makes the car veer.  Pick a
//  direction to nudge and how often (in ms):
//
//      0             -> no compensation (default; set once you
//                        know the drift direction)
//      10  ..  100    -> gentle correction every N ms
//      > 100          -> coarse / slow correction
//
//  See Motor.ino motorForward() for how it is used.
#define DRIFT_BIAS_MS       0    // <-- set to e.g. 30 once you
                                 //     have measured the drift


// -------------------------------------------------------------
//  4.  Shared state
// -------------------------------------------------------------
//  The current operating mode.  Updated by the button tab.
uint8_t currentMode = MODE_MANUAL;

//  Drift-bias timing.  `lastBiasMs` is the timestamp of the
//  last time we pulsed the slow wheel.  See applyDriftBias()
//  below for how it is used.
uint32_t lastBiasMs = 0;

//  Forward-motion flag.  Defined and maintained in Motor.ino
//  (set by motorForward(), cleared by every other motion
//  helper).  We forward-declare it here because the Arduino
//  IDE concatenates the tabs alphabetically -- Main.ino comes
//  before Motor.ino, so applyDriftBias() below would not see
//  the definition otherwise.
extern bool isMovingForward;


// -------------------------------------------------------------
//  5.  Arduino setup() and loop()
// -------------------------------------------------------------
void setup() {
  // Start the BT link first so any startup message is sent out.
  Serial.begin(9600);

  // Bring up every subsystem.
  motorSetup();
  sensorsSetup();
  buttonSetup();
  btSetup();

  // Park the car safely.
  motorStop();

  // Start in MANUAL mode (BT + voice work from the very first
  // second; a button press flips to AUTO).
  currentMode = MODE_MANUAL;

  Serial.println(F("READY: Manual mode (BT + voice)"));
}

// Forward-drift compensation.
//
//  The cheap DC gear motors used in a 2WD robot car almost
//  never spin at exactly the same RPM, so a perfectly
//  symmetric motorForward() makes the car veer.  This routine
//  applies a brief extra pulse to ONE wheel once every
//  DRIFT_BIAS_MS milliseconds to nudge the car back onto
//  course.  It only runs when isMovingForward is true (set by
//  motorForward(), cleared by every other motion helper).
//
//  Sign convention:
//      DRIFT_BIAS_MS > 0  -> car drifts LEFT  -> pulse RIGHT
//                              wheel (boost right speed)
//      DRIFT_BIAS_MS < 0  -> car drifts RIGHT -> pulse LEFT
//                              wheel
//      DRIFT_BIAS_MS == 0 -> disabled (default at boot)
//
//  The pulse is implemented as a brief "brake" blip on the
//  slow wheel, which slows it down for a moment and lets the
//  fast wheel catch up -- a crude but effective way to drift-
//  correct without ever needing PWM.
void applyDriftBias() {
  if (DRIFT_BIAS_MS == 0)       return;   // compensation off
  if (!isMovingForward)         return;   // not going forward
  if (currentMode != MODE_MANUAL) return;  // don't fight AUTO

  uint32_t now = millis();
  if ((int32_t)(now - lastBiasMs) < abs(DRIFT_BIAS_MS)) return;
  lastBiasMs = now;

  if (DRIFT_BIAS_MS > 0) {
    // Car drifts left -> slow the right wheel briefly.
    digitalWrite(IN3_PIN, LOW);  // right brake
    digitalWrite(IN4_PIN, HIGH);
    delay(20);                   // 20 ms blip is plenty
    digitalWrite(IN3_PIN, HIGH); // resume forward
    digitalWrite(IN4_PIN, LOW);
  } else {
    // Car drifts right -> slow the left wheel briefly.
    digitalWrite(IN1_PIN, LOW);  // left brake
    digitalWrite(IN2_PIN, HIGH);
    delay(20);
    digitalWrite(IN1_PIN, HIGH); // resume forward
    digitalWrite(IN2_PIN, LOW);
  }
}

void loop() {
  // Always poll the mode button first.
  currentMode = buttonUpdate(currentMode);

  // Dispatch to the right handler.
  if (currentMode == MODE_AUTO) {
    autonomousLoop();   // see Autonomous.ino
  } else {
    btHandle();         // see Bluetooth.ino  (works in manual mode)
  }

  // Forward-drift nudge, runs every iteration but only fires
  // when motorForward() is currently active and DRIFT_BIAS_MS
  // is non-zero.  See applyDriftBias() above.
  applyDriftBias();
}
