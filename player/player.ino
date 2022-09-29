#include "Audio.h"
#include <SD.h>
#include "FS.h"
#include <HortusWifi.h>
#include <NRotary.h>
#include <Button2.h>

#define DEBUG         1

#define SD_CS         5
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
#define MIN_VOLUME    15
#define MAX_TRACKNUM  0


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
_Rotary rotary2 = { 2, rtr2, IDLE, false };


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

int track_index = 0;


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
int volume = MIN_VOLUME;
int previous_volume = MIN_VOLUME;

String PLAYER_PLAY    = "/player/play";
String PLAYER_STOP    = "/player/stop";
String PLAYER_VOLUME  = "/player/volume";
String PLAYER_CHOOSE  = "/player/choose";
String AWAKE          = "/player/awake";


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

void longpress(Button2& btn) {
    unsigned int time = btn.wasPressedFor();
    Serial.print("You clicked ");
    if (time > 2000) {
        ESP.restart();
    }
}


void setup(){

    // SPI && SD SETUP
    
    pinMode(SD_CS, OUTPUT);
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
    button2.button.setLongClickHandler(longpress);

    // NETWORK && OSC SETUP
    
    HortusWifi(HortusWifi::Connection::HORTUS, 50, AWAKE);

    OscWiFi.subscribe(HortusWifi::RECV_PORT, PLAYER_PLAY,
    [](const OscMessage& m) {
        float _state = m.arg<float>(0);
        
        if (_state) 
            button1.state = true;

        _print("[ OSC RECEIVED ] ");
        _println(PLAYER_PLAY);

        send_osc((String&)m.address(), _state);
    });

    OscWiFi.subscribe(HortusWifi::RECV_PORT, PLAYER_STOP,
    [](const OscMessage& m) {
        float _state = m.arg<float>(0);
        
        if (_state) 
            button2.state = true;

        _print("[ OSC RECEIVED ] ");
        _println(PLAYER_STOP);

        send_osc((String&)m.address(), _state);
    });

    OscWiFi.subscribe(HortusWifi::RECV_PORT, PLAYER_VOLUME,
    [](const OscMessage& m) {
        float _vol = m.arg<float>(0);
        _vol = constrain(_vol, 0, MAX_VOLUME);
        
        if (_vol != previous_volume) {
            audio.setVolume(_vol);
            previous_volume = _vol;
        }  

        _print("[ OSC RECEIVED ] ");
        _println(PLAYER_VOLUME);

        send_osc((String&)m.address(), _vol);
    });

    OscWiFi.subscribe(HortusWifi::RECV_PORT, PLAYER_CHOOSE,
    [](const OscMessage& m) {
        int _choose = (int)m.arg<float>(0);

        if (_choose > 0) 
            _choose = 1;
        else
            _choose = -1;

        track_index += _choose;
        track_index = constrain(track_index, 0, MAX_TRACKNUM); 

        _print("[ OSC RECEIVED ] ");
        _println(PLAYER_CHOOSE);

        send_osc((String&)m.address(), _choose);
    });

    // AUDIO SETUP
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(volume); // 0...21
    //audio.connecttoFS(SD, "bsesample.wav");
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

    // check rotary1 (volume)
    check_volume(&rotary1);

    // check rotary2 (choose track)
    check_track(&rotary2);


    // check button1 (play)
    check_play(&button1);
    check_stop(&button2);
}

void check_volume(_Rotary *rot) 
{
    if (rot->state == CLOCKWISE)
    {
        volume--;
        volume = constrain(volume, 0, MAX_VOLUME);
        //Serial.print("CLOCKWISE ");
        Serial.print("[SET VOLUME] ");
        Serial.println(volume);
    }

    if (rot->state == COUNTER_CLOCKWISE)
    {
        volume++;
        volume = constrain(volume, 0, MAX_VOLUME);
        Serial.print("[SET VOLUME] ");
        Serial.println(volume);
    }

    if (volume != previous_volume) {
        audio.setVolume(volume);
        previous_volume = volume;
    }  
}

void check_track(_Rotary *rot) 
{
  if (rot->state == CLOCKWISE)
    {
        track_index--;
        track_index = constrain(track_index, 0, MAX_TRACKNUM);
        Serial.print("[ TRACK INDEX ] ");
        Serial.println(track_index);
    }

    if (rot->state == COUNTER_CLOCKWISE)
    {
        track_index++;
        track_index = constrain(track_index, 0, MAX_TRACKNUM);
        Serial.print("[ TRACK INDEX ] ");
        Serial.println(track_index);
    }   
}

void check_play(Button *b)
{
    if (b->state) {
        play_audio(notes[track_index]);
        b->state = false;   
    }          
}

void check_stop(Button *b)
{
    if (b->state) {
        stop_audio();
        b->state = false;
    }    
}

void play_audio(String audiofile)
{
    int old_volume = volume;
    
    while (volume) { 
        volume--;
        audio.setVolume(volume);
    }

    audio.connecttoFS(SD, audiofile.c_str());
    _println("[ PLAYING AUDIO ]");

    while (volume <= old_volume) {
      volume++;
      audio.setVolume(volume); 
    }
}

void stop_audio()
{
    audio.stopSong();
    _println("[ STOPPING AUDIO ]");
}

void send_osc(String& addr, int value)
{
    OscWiFi.send(HortusWifi::HOST, HortusWifi::SEND_PORT, addr, value);
}

int _print(String s)
{
#if DEBUG
    Serial.print(s);
#endif
    return 0;
}

int _println(String s)
{
#if DEBUG
    Serial.println(s);
#endif
    return 0;
}
