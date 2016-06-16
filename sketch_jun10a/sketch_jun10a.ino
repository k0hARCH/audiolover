// Using :
// - DualMoco : http://morecatlab.akiba.coocan.jp/lab/index.php/aruino/midi-firmware-for-arduino-uno-moco/
// - MIDI Library : http://playground.arduino.cc/Main/MIDILibrary
// 
// Device:
// LED    : D13
// BUTTON : D8 (Note ON/OFF)
// VOLUME : A0 (MIDI Note) 
// CDS    : A1 (MIDI PitchBend) 

#include <MIDI.h>

MIDI_CREATE_DEFAULT_INSTANCE();

// つなげる場所により変えよう
const int LED = 13;
const int BUTTON = 8;
const int VOLUME = 0;
const int CDS = 1;

// 回路の抵抗度合いなどに応じて数値は調整しよう
#define note_MIN 0
#define note_MAX 127
#define vol_MIN 200 // volume保護用に抵抗で分圧してる関係上
#define vol_MAX 1023

// これもCdSを手で覆った時だけ使いたいのでこんな値
// 明るさによって変わるので調整しよう
#define cds_MIN 820
#define cds_MAX 1023

// pitchbendは本当は -8192 - 8191 だけど
// 元々の値が荒いので、ベンド幅を1/4にしとく。
// これだとあまり変わらないので、もっと増やしてもいいかも。
// そのへんは好みでw
#define pb_MIN -2048
#define pb_MAX 2048

// 初期化処理です
void setup(){
  pinMode(LED,OUTPUT); 
  pinMode(BUTTON,INPUT); 
  MIDI.begin(1);                // Launch MIDI with default options
  MIDI.turnThruOff();
}

// これはセンサー入力のフラつき対策
boolean checkChange(int val1,int val2, int range){
  int l,h;
  l = val2-range;
  h = val2+range;
  if((l > val1)||(h < val1)){
   return true;
  }
  return false;
} 

int button = 0;
int note = 0;
int cds = 0;
boolean isplay = false;

void loop(){
  int nowbutton;
  int nownote;
  int nowcds;

  // Note Number change
  nownote = analogRead(VOLUME);    
  nownote = map(nownote, vol_MAX, vol_MIN, note_MIN, note_MAX);
  if(checkChange(nownote,note,1)){
    if(isplay){
      MIDI.sendNoteOff(note,0,1);
      MIDI.sendNoteOn(nownote,100,1);
    }
    note = nownote;
  }

  // Cds (Pitchbend)
  nowcds = analogRead(CDS);
  if(nowcds > cds_MIN){
    nowcds = map(nowcds, cds_MIN, cds_MAX, pb_MAX, pb_MIN);
    MIDI.sendPitchBend(nowcds,1);
  }else{
    nowcds = 0;
    if(cds != nowcds){
      MIDI.sendPitchBend(0,1);
    }     
  }
  cds = nowcds;

  // Button (Note ON/OFF)
  nowbutton = digitalRead(BUTTON);
  if(nowbutton != button){
    if(nowbutton == HIGH) {
      MIDI.sendNoteOn(note,100,1);
      isplay = true;
      digitalWrite(LED,HIGH);
    }else{
      MIDI.sendNoteOff(note,0,1);
      isplay = false;
      digitalWrite(LED,LOW);
    }
    delay(50);
  }
  button = nowbutton;
}
