// =============================================================
//  Bluetooth.ino  --  HC-05 command parser
//  Tab 5 of 7
//
//  This file handles ALL commands received from the HC-05
//  Bluetooth module, in both BT mode and voice mode.  The
//  difference between the two is only how the data looks:
//
//      BT mode (car-control app):
//          F  -> Go Ahead
//          B  -> Backward
//          L  -> Left
//          R  -> Right
//          S  -> Stop
//          (One character per command, usually followed by
//           a newline, but works without one too.)
//
//      Voice mode (voice-control app):
//          go ahead | backward | back | left | right | stop
//          (Words; terminated by '\r' or '\n'.)
//
//  Both styles are normalised into a word and then mapped to
//  a motor action through the same wordToCommand() lookup, so
//  there is only one place to add or rename commands.
// =============================================================


// One-time setup hook for the Bluetooth tab.  Currently a
// no-op because Serial.begin(9600) is already called from
// setup() in Main.ino, but we keep the function so other
// tabs have a single, consistent place to initialise BT
// features later on.
void btSetup() {
  // intentionally empty
}


// Forward lookup: a spoken word -> the matching single-letter
// action code.  Returns 0 if the word is not recognised.
char wordToCommand(String word) {
  if (word == "GOAHEAD")    return 'F';
  if (word == "GO AHEAD")   return 'F';
  if (word == "GO")         return 'F';   // short alias
  if (word == "AHEAD")      return 'F';   // short alias
  if (word == "FORWARD")    return 'F';   // legacy alias
  if (word == "BACKWARD")   return 'B';
  if (word == "BACK")       return 'B';   // short alias for BACKWARD
  if (word == "LEFT")       return 'L';
  if (word == "RIGHT")      return 'R';
  if (word == "STOP")       return 'S';
  return 0;   // unknown word
}


// Reverse lookup: a single BT-style letter -> the matching
// word.  Used so a one-character BT command ("F") and a voice
// command ("GO AHEAD") both end up at the same mapping step --
// the word -> action lookup in wordToCommand().  Change the
// word list here and in wordToCommand() and both apps pick
// it up.
String letterToWord(char c) {

  switch (c) {
    case 'F': return String("FORWARD");
    case 'D': return String("FORWARD");   // alias used by some apps
    case 'B': return String("BACKWARD");
    case 'L': return String("LEFT");
    case 'E': return String("LEFT");      // alias used by some apps
    case 'R': return String("RIGHT");
    case 'S': return String("STOP");
    default:  return String();   // empty -> unknown
  }
}


// Run a single-letter command (F B L R S).  Anything else
// is silently ignored.
void runLetterCommand(char c) {
  switch (c) {
    case 'F': motorForward();  break;
    case 'B': motorBackward(); break;
    case 'L': motorLeft();     break;
    case 'R': motorRight();    break;
    case 'S': motorStop();     break;
  }
}


// Read one command (BT letter or voice word) from the serial
// buffer.  BT letters and voice words are both normalised to
// a word, then looked up in wordToCommand().
//
// Returns true if a command was found and applied (or
// silently ignored because it was unknown); false if no data
// is waiting.
bool btReadCommand() {
  if (!Serial.available()) return false;

  // Read until the first whitespace / terminator.  The BT
  // app sends one letter + newline; the voice app sends a
  // whole word + newline.  Either way, the first chunk is
  // a complete command.
  String word;
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\r' || c == '\n' || c == ' ' || c == '\t') break;
    word += (char)toupper(c);
  }
  if (word.length() == 0) return true;   // empty line, ignore

  // If the chunk was a single BT-style letter, expand it to
  // its full word so both paths go through the same lookup.
  if (word.length() == 1) {
    String full = letterToWord(word[0]);
    if (full.length() > 0) word = full;
  }

  char cmd = wordToCommand(word);
  if (cmd != 0) {
    runLetterCommand(cmd);
  }
  return true;
}


// Main entry point.  Called from loop() every iteration.
void btHandle() {
  btReadCommand();
}
