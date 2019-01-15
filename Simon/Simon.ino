#include <pitches.h>
#include <Adafruit_NeoPixel.h>

#define PIN      6
#define N_LEDS 4
#define YELLOW 0
#define GREEN 1
#define RED 2
#define BLUE 3
int button = 9;
 
// notes in the melody:
int melody[] = {
  NOTE_C5, NOTE_D5, NOTE_E5, NOTE_F5, NOTE_G5, NOTE_A5, NOTE_B5, NOTE_C6};
int duration = 500;  // 500 miliseconds

static int code[6] = {RED, RED, YELLOW, BLUE, YELLOW, GREEN};
static int codeNotes[6] = {NOTE_A4, NOTE_A4, NOTE_C4, NOTE_E4, NOTE_C4, NOTE_E3};

Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_LEDS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  pinMode(button, INPUT_PULLUP);  
  strip.begin();
}

void loop() {
  if (digitalRead(button) == LOW)
  {
    for (int i = 0; i < 6; i++) {
      play_note(code[i], codeNotes[i], strip.Color(255, 255, 255));
      delay(500);
    }
  }
}

static void play_note(int index, int note, uint32_t c){
    strip.setPixelColor(index, c); // Turn on
    strip.show();
      
    tone(8, note, duration);
    delay(500);
    
    strip.setPixelColor(index, 0); // Erase pixel 
    strip.show();
}
