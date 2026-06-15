#define FRONT_IR D5

void setup() {
  Serial.begin(115200);
  pinMode(FRONT_IR, INPUT);

  Serial.println("Front IR Sensor Test");
}

void loop() {
  int state = digitalRead(FRONT_IR);

  Serial.print("IR State: ");
  Serial.println(state);

  delay(300);
}