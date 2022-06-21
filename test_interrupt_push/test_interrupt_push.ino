#define PUSH 13
#define PRESS 0
#define RELEASE 1

struct Button {
  const uint8_t PIN;
  uint8_t previous;
};

Button button = {PUSH, 1};

void setup() {
  Serial.begin(115200);
  pinMode(button.PIN, INPUT_PULLUP);
}

void loop() {
  uint8_t reading = digitalRead(button.PIN);

  if (reading == PRESS) {
    if (button.previous == RELEASE) {
      Serial.println("released");
      delay(200);
    }
  } 
  
  button.previous = reading;
}
