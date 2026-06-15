// =============================================================
//  Notes.ino  --  Reference / cheat sheet
//  Tab 7 of 7
//
//  This file is intentionally just a long comment block.
//  It does not contain any code, so it has no effect on the
//  compiled binary.  Use it as a quick reference.
//
//  ---------------- HARDWARE WIRING ----------------
//  L298N motor driver:
//      IN1 -> D1        Motor A (left)  forward
//      IN2 -> D2        Motor A (left)  reverse
//      IN3 -> D7        Motor B (right) forward
//      IN4 -> D8        Motor B (right) reverse
//      ENA / ENB jumpers left ON the board
//      GND  -> common ground
//      +12V -> battery positive
//
//  HC-05 Bluetooth module:
//      TXD -> NodeMCU RX
//      RXD -> NodeMCU TX    (disconnect during USB upload)
//      VCC -> 5V
//      GND -> GND
//
//  HC-SR04 ultrasonic sensor:
//      TRIG -> D0
//      ECHO -> D6   (1k -> D6, 2k -> GND voltage divider)
//      VCC  -> 5V
//      GND  -> GND
//
//  IR cliff sensor (front-facing):
//      OUT -> D5   (HIGH = cliff,  LOW = floor)
//      VCC -> 3.3 V
//      GND -> GND
//
//  Mode button:
//      D3 <-> GND,  uses INPUT_PULLUP
//      (release before power-up / reset / flash upload)
//
//  ---------------- SUPPORTED COMMANDS ----------------
//  BT mode  (car-control app, single chars):
//      F  B  L  R  S
//
//  Voice mode  (voice-control app, words, case-insensitive):
//      forward
//      backward / back
//      left
//      right
//      stop
//
//  Mode button  (D3, toggles):
//      press once  -> switch between Manual and Auto
//
//  ---------------- MODE STATES ----------------
//  On boot the car is in MANUAL mode: it accepts BT and
//  voice commands.  Press the button once to switch to AUTO
//  (the car drives itself).  Press again to come back to
//  MANUAL.  The current state is printed to the Serial
//  monitor at 9600 baud and also echoed to the BT link.
// =============================================================
