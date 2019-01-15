#include <Wire.h> // Enable this line if using Arduino Uno, Mega, etc.
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
// Make sure the file pitches.h is placed in the same folder as this sketch
#include "pitches.h"
#define ALARMSOUNDER 12
#define COUNT_DOWN 1
#define COUNT_UP 2
#define PLAY_BUZZER 3
int state = COUNT_DOWN;

uint16_t minutes = 60;
uint16_t seconds = 0;
Adafruit_7segment matrix = Adafruit_7segment();

// Piezzo element connected to Arduino pin 12 and ground
const int buzzerPin = 12;

// Array with the notes in the melody (see pitches.h for reference)
int melody[] = {NOTE_A4, NOTE_A4, NOTE_A4, NOTE_F4, NOTE_C5, NOTE_A4, NOTE_F4, NOTE_C5, NOTE_A4, NOTE_E5, NOTE_E5, NOTE_E5, NOTE_F5, NOTE_C5, NOTE_A4, NOTE_F4, NOTE_C5, NOTE_A4};

// Array with the note durations: a quarter note has a duration of 4, half note 2 etc.
int durations[]  = {4, 4, 4, 5, 16, 4, 5, 16, 2, 4, 4, 4, 5, 16, 4, 5, 16, 2};

int tempo = 120; // tempo for the melody expressed in beats per minute (BPM)

void setup() {
  Serial.begin(9600);
  Serial.println("7 Segment Backpack Test");
  //pinMode(buzzerPin,OUTPUT);//initialize the buzzer pin as an output
 
  matrix.begin(0x70);
}

void loop() {
  uint16_t current_time = minutes*100 + seconds;
  Serial.print("Time: ");
  Serial.print(current_time);
  Serial.print(" Minutes: ");
  Serial.print(minutes);
  Serial.print(" Seconds: ");
  Serial.println(seconds);
  matrix.println(current_time);
  matrix.drawColon(true);
  matrix.writeDisplay();
  delay(1000);

  // Time's up
  if (seconds == 0 && minutes == 0) {
    playTune(melody, durations, tempo);
/*    //output an frequency
    for(int i=0;i<80;i++)
    {
      digitalWrite(buzzer,HIGH);
      delay(1);//wait for 1ms
      digitalWrite(buzzer,LOW);
      delay(1);//wait for 1ms
    }
  
    //output another frequency
    for(int i=0; i<100; i++)
    {
      digitalWrite(buzzer,HIGH);
      delay(2);//wait for 2ms
      digitalWrite(buzzer,LOW);
      delay(2);//wait for 2ms
    }
    */
    state = COUNT_UP;
    seconds = 1;
    return;
  }

  if (state == COUNT_DOWN) {
    if (seconds == 0) {
      minutes--;
      seconds = 59;
    } else {
      seconds--;
    }
  }
  
  if (state == COUNT_UP) {
    if (seconds == 59) {
      minutes++;
      seconds = 0;
    } else {
      seconds++;
    }
  }
}

void playTune(int notes[], int durations[], int BPM) 
{
  int tuneSize = sizeof(melody) / sizeof(int);
  
  // iterate over the notes of the tune:
  for (int thisNote = 0; thisNote < tuneSize; thisNote++) {

    // For details on calculating the note duration using the tempo and the note type,
    // see http://bradthemad.org/guitar/tempo_explanation.php.
    // A quarter note at 60 BPM lasts exactly one second and at 120 BPM - half a second.
    
    int noteDuration = (int)((1000 * (60 * 4 / BPM)) / durations[thisNote] + 0.);
    tone(buzzerPin, notes[thisNote],noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 20% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.20;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(buzzerPin);
  }
}

void buzz() {
  int i, j;
  for (j = 0; j < 10; j++) {
    
    for(i = 0; i < 255; i = i + 2)
    {
        analogWrite(ALARMSOUNDER, i);
        delay(10);
    }
    for(i = 255; i > 1; i = i - 2)
    {
        analogWrite(ALARMSOUNDER, i);
        delay(5);
    }
    for(i = 1; i <= 10; i++)
    {
        analogWrite(ALARMSOUNDER, 200);
        delay(100);
        analogWrite(ALARMSOUNDER, 25);
        delay(100);
    }
  }
}
