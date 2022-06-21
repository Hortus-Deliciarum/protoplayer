#include "Audio.h"
#include <SD.h>
#include "FS.h"

// Digital I/O used
#define SD_CS          5
#define SPI_MOSI      19
#define SPI_MISO      23
#define SPI_SCK       18
#define I2S_DOUT      25
#define I2S_BCLK      27
#define I2S_LRC       26

#define PUSH 13
#define PRESS 0
#define RELEASE 1

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
  
uint8_t counter = 0;

struct Button {
  const uint8_t PIN;
  uint8_t previous;
};

Button button = {PUSH, 1};

Audio audio;

void setup(){
    pinMode(SD_CS, OUTPUT);
    pinMode(button.PIN, INPUT_PULLUP);
    digitalWrite(SD_CS, HIGH);
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    Serial.begin(115200);
    SD.begin(SD_CS);
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(10); // 0...21
    //audio.connecttoFS(SD, "break2.wav");
}

void loop(){
    audio.loop();
    uint8_t reading = digitalRead(button.PIN);
    // audio.connecttoFS(SD, "test.wav");

    if (reading == PRESS) {
    if (button.previous == RELEASE) {
      audio.connecttoFS(SD, notes[counter].c_str());
      counter++;
      if (counter > 7)
        counter = 0;
      delay(200);
    }
  } 
  
  button.previous = reading;
    
}
