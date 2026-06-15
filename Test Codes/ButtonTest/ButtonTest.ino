// Pin definition for NodeMCU
const int BUTTON_PIN = D3; // Built-in FLASH button (GPIO0)

// Time tracking configuration (in milliseconds)
const unsigned long TIME_WINDOW = 2000; // 2 seconds to press as fast as you can

// Variables for state, counting, and timing
int lastButtonState = HIGH; 
int pressCount = 0;
unsigned long windowStartTime = 0;
bool isTrackingTime = false;

void setup() {
  Serial.begin(115200);
  delay(10);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  Serial.println("\n--- Rapid Press Counter Started ---");
  Serial.println("Press the FLASH button to start the 2-second timer!");
}

void loop() {
  int currentButtonState = digitalRead(BUTTON_PIN);

  // Detect a distinct button press (HIGH to LOW)
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    
    // If this is the FIRST press, start the countdown timer
    if (!isTrackingTime) {
      isTrackingTime = true;
      windowStartTime = millis();
      pressCount = 1; // Count this first press
      Serial.println("\n⏱️ Timer Started! Mash the button now!");
    } 
    // If the timer is already running, just add to the count
    else {
      pressCount++;
      Serial.print("Hits: ");
      Serial.println(pressCount);
    }
    
    delay(120); // Small debounce delay to prevent fake double-triggers
  }

  lastButtonState = currentButtonState;

  // Check if the timer is running AND if the 2 seconds have expired
  if (isTrackingTime && (millis() - windowStartTime >= TIME_WINDOW)) {
    // Stop the timer
    isTrackingTime = false; 
    
    // Print the final result
    Serial.println("🛑 TIME IS UP!");
    Serial.print("👉 Total presses in 2 seconds: ");
    Serial.println(pressCount);
    Serial.println("\nPress the button again to start a new test!");
  }
}