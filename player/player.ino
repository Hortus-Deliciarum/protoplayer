#include "Audio.h"
#include <SD.h>
#include "FS.h"
//#include "ArduinoOSCWiFi.h"
#include <HortusWifi.h>
#include <NRotary.h>
#include <Button2.h>


// Digital I/O used
#define SD_CS          5
#define SPI_MOSI      19
#define SPI_MISO      23
#define SPI_SCK       18
#define I2S_DOUT      25
#define I2S_BCLK      27
#define I2S_LRC       26

#define PUSH          13
#define PUSH2         15

#define PRESS         0
#define RELEASE       1
#define MAX_VOLUME    21
#define DEF_VOLUME    15


#define ROTARY1_PIN1 14
#define ROTARY1_PIN2 12

#define ROTARY2_PIN1 4
#define ROTARY2_PIN2 0

bool usingInterrupt = true;

typedef struct {
  byte number;
  Rotary& rotary; 
  int state;
  bool _switch; 
} _Rotary;

Rotary rtr1 = Rotary(ROTARY1_PIN1, ROTARY1_PIN2, usingInterrupt, true, INPUT_PULLUP, 50);
Rotary rtr2 = Rotary(ROTARY2_PIN1, ROTARY2_PIN2, usingInterrupt, true, INPUT_PULLUP, 50);

_Rotary rotary1 = { 1, rtr1, IDLE, false };
_Rotary rotary2 = { 2, rtr1, IDLE, false };


typedef struct {
  byte number;
  Button2& button;
  bool state;
} Button;

Button2 btn1, btn2;

Button button1 = { 1, btn1, false };
Button button2 = { 2, btn2, false }; 

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


/*
int rotary1State = IDLE;
bool rotary1Switch = false;
*/

//Interrupt Service Routine(ISR).
void rotary1ServiceRoutineWrapper()
{
    //Run rotary1.serviceRoutine() here.
    rotary1.rotary.serviceRoutine();
}

//Interrupt Service Routine(ISR).
void rotary2ServiceRoutineWrapper()
{
    //Run rotary1.serviceRoutine() here.
    rotary2.rotary.serviceRoutine();
}

uint8_t counter = 0;
int volume = DEF_VOLUME;
int previous_volume = DEF_VOLUME;
String AWAKE = "/player/awake";


Audio audio;

void onOscReceived(const OscMessage& m) {
    volume = (int)m.arg<float>(0);
    volume = constrain(volume, 0, MAX_VOLUME);
    audio.setVolume(volume);
}


void released(Button2& btn) {
    if (btn == button1.button) {
      Serial.println("released PUSH1");
      button1.state = true; 
    } else if (btn == button2.button) {
      Serial.println("released PUSH2");
      button2.state = true; 
    }  
}


void setup(){

    // SPI && SD SETUP
    pinMode(SD_CS, OUTPUT);
    //pinMode(button.PIN, INPUT_PULLUP);
    digitalWrite(SD_CS, HIGH);
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    Serial.begin(115200);
    SD.begin(SD_CS);

    // ROTARY && BUTTON SETUP
    attachInterrupt(digitalPinToInterrupt(ROTARY1_PIN1), rotary1ServiceRoutineWrapper, rotary1.rotary.mode);
    attachInterrupt(digitalPinToInterrupt(ROTARY2_PIN1), rotary2ServiceRoutineWrapper, rotary2.rotary.mode);
    
    button1.button.begin(PUSH);
    button1.button.setReleasedHandler(released);

    button2.button.begin(PUSH2);
    button2.button.setReleasedHandler(released);

    // NETWORK && OSC SETUP
    HortusWifi(HortusWifi::Connection::BARETTI, 50, AWAKE);
    OscWiFi.subscribe(HortusWifi::RECV_PORT, "/callback", onOscReceived);    

    // AUDIO SETUP
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(volume); // 0...21
    audio.connecttoFS(SD, "bsesample.wav");
}

void check_volume(_Rotary& rot) {
    if (rot->state == CLOCKWISE)
    {
        volume--;
        volume = constrain(volume, 0, MAX_VOLUME);
        Serial.print("CLOCKWISE ");
        Serial.println(volume);
    }

    if (rot->state == COUNTER_CLOCKWISE)
    {
        volume++;
        volume = constrain(volume, 0, MAX_VOLUME);
        Serial.print("COUNTER-CLOCKWISE ");
        Serial.println(volume);
    }

    if (volume != previous_volume) {
      audio.setVolume(volume);
      previous_volume = volume;
    }  
}

void loop(){
    OscWiFi.update();
    audio.loop();
    button1.button.loop();
    button2.button.loop();

    rotary1.state = rotary1.rotary.getState();
    rotary2.state = rotary2.rotary.getState();

    //Save the state of the rotary1 switch.
    rotary1._switch = rotary1.rotary.getSwitch();
    rotary2._switch = rotary2.rotary.getSwitch();

    check_volume(rotary1);

    if (button1.state) {
      audio.setVolume(0);
      audio.connecttoFS(SD, notes[counter].c_str());
      audio.setVolume(volume);
      button1.state = false; 
      counter++;
      if (counter > 7)
        counter = 0;   
    }
}
