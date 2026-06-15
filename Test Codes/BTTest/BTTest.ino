void setup() {
  Serial.begin(9600);   // HC-05 default baud rate
}

void loop() {
  // Data from Phone → HC-05 → ESP8266
  if (Serial.available()) {
    char c = Serial.read();

    Serial.print("Received: ");
    Serial.println(c);
  }
}