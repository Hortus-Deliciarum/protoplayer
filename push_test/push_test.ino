#define PUSH1   13
#define PUSH2   15
#define N_PUSH  2
#define LED     33
#define NOT_PUSHED  1
#define PUSHED      0

typedef struct {
  uint8_t pin;
  uint8_t previous;  
} t_push;

t_push push[N_PUSH];
uint8_t values[N_PUSH] = { NOT_PUSHED, NOT_PUSHED };

void setup() {
  Serial.begin(115200);

  push[0].pin = PUSH1;
  push[1].pin = PUSH2;

  for (int i=0; i<N_PUSH; i++) {
    push[i].previous = NOT_PUSHED;
    pinMode(push[i].pin, INPUT_PULLUP);
  }

  pinMode(LED, OUTPUT);
}

void loop() { 
  for (int i=0; i<N_PUSH; i++) {
    values[i] = digitalRead(push[i].pin);
    if (push[i].previous != values[i]) {
      
      if (values[i] == PUSHED)
         digitalWrite(LED, digitalRead(LED) ^ 1);
      push[i].previous = values[i];  
    }  
  }  

   delay(1);
}
