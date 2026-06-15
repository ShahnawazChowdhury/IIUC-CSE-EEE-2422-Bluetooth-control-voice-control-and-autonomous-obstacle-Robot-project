// =============================================================
//  Button.ino  --  Mode toggle on the D3 push button
//  Tab 4 of 7
//
//  Wiring: one leg of the button -> D3, the other leg -> GND.
//  We use the internal pull-up resistor, so the pin reads
//  HIGH when the button is released and LOW when it is pressed.
//
//  IMPORTANT:
//      D3 (GPIO0) is also the ESP8266's boot strap pin.  If it
//      is held LOW at power-up / reset / during a USB flash,
//      the chip will not boot.  Always release the button
//      before powering the board on or uploading new code.
//
//  Behaviour:
//      Each clean press of the button toggles the operating
//      mode:
//          MANUAL  <->  AUTO
// =============================================================


// Internal state for debouncing.
bool     _btnStable   = HIGH;   // last stable reading
bool     _btnRaw      = HIGH;   // most recent raw reading
uint32_t _btnChangeMs = 0;      // when the raw reading last changed


// Initialise the button pin and debounce state.
void buttonSetup() {
  pinMode(BTN_PIN, INPUT_PULLUP);
  _btnStable   = digitalRead(BTN_PIN);
  _btnRaw      = _btnStable;
  _btnChangeMs = millis();
}


// Call this once per loop().  Returns the (possibly updated)
// current mode.
uint8_t buttonUpdate(uint8_t mode) {
  // 1. Read the raw pin.
  bool    raw = digitalRead(BTN_PIN);
  uint32_t now = millis();

  // 2. Track the moment the raw value last changed.
  if (raw != _btnRaw) {
    _btnRaw      = raw;
    _btnChangeMs = now;
  }

  // 3. If the new value has been stable for long enough,
  //    accept it and fire on the press edge (HIGH -> LOW).
  if ((now - _btnChangeMs) >= BUTTON_DEBOUNCE_MS && raw != _btnStable) {
    _btnStable = raw;

    if (_btnStable == LOW) {
      // Toggle between MANUAL and AUTO.
      if (mode == MODE_MANUAL) mode = MODE_AUTO;
      else                    mode = MODE_MANUAL;

      Serial.print(F("MODE: "));
      Serial.println(mode == MODE_AUTO ? F("Auto") : F("Manual"));
    }
  }

  return mode;
}
