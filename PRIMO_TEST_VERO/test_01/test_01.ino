#include "Audio.h"
#include <SD.h>
#include "FS.h"
#include <ArduinoOSCWiFi.h>
#include <NRotary.h>


// Digital I/O used
#define SD_CS          5
#define SPI_MOSI      19
#define SPI_MISO      23
#define SPI_SCK       18
#define I2S_DOUT      25
#define I2S_BCLK      27
#define I2S_LRC       26
#define PUSH          13
#define PRESS         0
#define RELEASE       1


#define ROTARY_PIN1 14
#define ROTARY_PIN2 12

Rotary rotary = Rotary(ROTARY_PIN1, ROTARY_PIN2, PUSH, true, INPUT_PULLUP, 50);


String notes[8] = {
  "01.wav",
  "02.wav",
  "03.wav",
  "04.wav",
  "05.wav",
  "06.wav",
  "07.wav",
  "08.wav"
  };

//Where to save rotary turning state.
int rotaryState = IDLE;

//Where to save rotary switch state.
bool rotarySwitch = false;

//Interrupt Service Routine(ISR).
void rotaryServiceRoutineWrapper()
{
    //Run rotary.serviceRoutine() here.
    rotary.serviceRoutine();
}

uint8_t counter = 0;

struct Button {
  const uint8_t PIN;
  uint8_t previous;
};

Button button = {PUSH, 1};
int volume = 10;
int previous_volume = 10;

const char* ssid = "FASTWEB-DAB6F7";
const char* password = "1NJYRZE2T4";
const int recv_port = 54321;

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

Audio audio;

void onOscReceived(const OscMessage& m) {
    volume = (int)m.arg<float>(0);
    volume = constrain(volume, 0, 21);
    audio.setVolume(volume);
}

void setup(){
    pinMode(SD_CS, OUTPUT);
    pinMode(button.PIN, INPUT_PULLUP);
    digitalWrite(SD_CS, HIGH);
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    Serial.begin(115200);
    SD.begin(SD_CS);

    attachInterrupt(digitalPinToInterrupt(ROTARY_PIN1), rotaryServiceRoutineWrapper, rotary.mode);

    initWiFi();
    OscWiFi.subscribe(recv_port, "/callback", onOscReceived);    
    
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(volume); // 0...21
    audio.connecttoFS(SD, "see.wav");
}

void loop(){
    OscWiFi.parse();
    audio.loop();

    rotaryState = rotary.getState();

    //Save the state of the rotary switch.
    rotarySwitch = rotary.getSwitch();

    if (rotaryState == CLOCKWISE)
    {
        volume--;
        volume = constrain(volume, 0, 21);
        Serial.print("CLOCKWISE ");
        Serial.println(volume);
    }

    if (rotaryState == COUNTER_CLOCKWISE)
    {
        volume++;
        volume = constrain(volume, 0, 21);
        Serial.print("COUNTER-CLOCKWISE ");
        Serial.println(volume);
    }

    if (volume != previous_volume) {
      audio.setVolume(volume);
      previous_volume = volume;
    }
    
    uint8_t reading = digitalRead(button.PIN);    

    if (reading == PRESS) {
      if (button.previous == RELEASE) {
        audio.connecttoFS(SD, notes[counter].c_str());
        counter++;
        if (counter > 7)
          counter = 0;
        delay(10);
      }
    } 
  
  button.previous = reading;
    
}
