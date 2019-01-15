#include <PS2Keyboard.h>
#include <Adafruit_NeoPixel.h>

#define ALPHA_PIN    6
#define TUBE_PIN     5
#define N_ALPHA_LEDS 100
#define N_TUBE_LEDS  5
#define reed   A0 //pin connected to read switch

const int DataPin = 8;
const int IRQpin =  3;
//                             a   b   c   d   e   f   g   h   i   j   k   l   m   n   o   p   q   r   s   t   u   v   w   x   y   z
const int letterLocation[] = {66, 69, 73, 76, 80, 84, 88, 91, 57, 54, 51, 49, 46, 43, 40, 37, 34,  1,  4,  7, 10, 12, 16, 20, 23, 26};
const uint32_t codes[]     = {'f','m','s','h','a','j','t','b','n','x','y','u','i','o','p','l','k','q','w','g','d','z','e','r','c','v'};
const float encodeMph = 10.0;

// Pin 13 has an LED connected on most Arduino boards.
// give it a name:
int redLed = 11;
int greenLed = 12;

PS2Keyboard keyboard;
Adafruit_NeoPixel alpha_strip = Adafruit_NeoPixel(N_ALPHA_LEDS, ALPHA_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel tube_strip = Adafruit_NeoPixel(N_TUBE_LEDS, TUBE_PIN, NEO_GRB + NEO_KHZ800);
int incomingByte = 0;   // for incoming serial data
bool showWelcome = true;
int letterSpacing = 3;
int letterOffset = 12;

//calculations
//tire radius ~ 13.5 inches
//circumference = pi*2*r =~85 inches
//max speed of 35mph =~ 616inches/second
//max rps =~7.25

//storage variables (cycle test)
int reedVal;
long timer; // time between one full rotation (in ms)
float mph;
float radius = 8.0; // tire radius (in inches)
float circumference;

int maxReedCounter = 100; //min time (in ms) of one rotation (for debouncing)
int reedCounter;

// Reed must open and close to indicate motion - prevent mph if reed is held open by the magnet
int reedClosedCounter = 0;
int maxReedClosedCount = 50; // If reed is closed longer than this, it's not moving (fast enough)

void setup() {
  delay(1000);

  reedCounter = maxReedCounter;
  circumference = 2*3.14*radius;
  pinMode(reed, INPUT);

  // initialize the digital pin as an for LEDs
  pinMode(redLed, OUTPUT);     
  pinMode(greenLed, OUTPUT);  
  
  // TIMER SETUP- the timer interrupt allows precise timed measurements of the reed switch
  //for more info about configuration of arduino timers see http://arduino.cc/playground/Code/Timer1
  cli();//stop interrupts

  //set timer1 interrupt at 1kHz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;
  // set timer count for 1khz increments
  OCR1A = 1999;// = (1/1000) / ((1/(16*10^6))*8) - 1
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS11 bit for 8 prescaler
  TCCR1B |= (1 << CS11);   
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  
  sei();//allow interrupts
  //END TIMER SETUP
  
  keyboard.begin(DataPin, IRQpin);
  Serial.begin(9600);
  alpha_strip.begin();
  tube_strip.begin();
  Serial.println("Keyboard Test:");
}

void loop() {
  if (mph > encodeMph) {
    digitalWrite(redLed, LOW);   // turn the LED off (LOW is the voltage level)
    digitalWrite(greenLed, HIGH);   // turn the LED on (HIGH is the voltage level)
  } else {
    digitalWrite(redLed, HIGH);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(greenLed, LOW);   // turn the LED off (LOW is the voltage level)
  }

  updateTubeLights(mph/encodeMph);
  
  if (keyboard.available()) {
    // read the next key
    char c = keyboard.read();
    Serial.println(c);
    int letter = c;

    // Make lower if needed
    if (c >= 'A' && c <= 'Z') {
      c = 'a' + (c - 'A');
    }
  
    Serial.print(c);
    Serial.print(" = ");
    if (mph > encodeMph) {
      letter = encodeLetter(c);
    } 
    
    Serial.println((char)letter);

    if (letter >= 'a' && letter <= 'z') {  
      lightLetter((letter), alpha_strip.Color(255, 255, 255));       
    }
  }
}

ISR(TIMER1_COMPA_vect) {//Interrupt at freq of 1kHz to measure reed switch
  reedVal = digitalRead(reed);//get val of A0
  if (reedVal){//if reed switch is closed
    if (reedClosedCounter > maxReedClosedCount) { //reed is being held open by the magnet
      mph = 0;
    } else {
      reedClosedCounter += 1;
      if (reedCounter == 0){//min time between pulses has passed
        mph = (56.8*float(circumference))/float(timer);//calculate miles per hour
        timer = 0;//reset timer
        reedCounter = maxReedCounter;//reset reedCounter
      }
      else{
        if (reedCounter > 0){//don't let reedCounter go negative
          reedCounter -= 1;//decrement reedCounter
        }
      }
    }
  }
  else{//if reed switch is open
    reedClosedCounter = 0; //Reed has opened, reset the closed count
    if (reedCounter > 0){//don't let reedCounter go negative
      reedCounter -= 1;//decrement reedCounter
    }
  }
  if (timer > 2000){
    mph = 0;//if no new pulses from reed switch- tire is still, set mph to 0
  }
  else{
    timer += 1;//increment timer
  } 
}

static void lightLetter(uint16_t letter, uint32_t c) {

  Serial.print("Lighting: ");
  Serial.print(letter);
  Serial.print(" Index is: ");
  Serial.println(letterLocation[letter-'a']);
  
  if (letter < 'a' || letter > 'z' ) {
    return;
  }
  int i = letter - 'a';
  alpha_strip.setPixelColor(letterLocation[i], c);
  alpha_strip.show();
  delay(1000);
  alpha_strip.setPixelColor(letterLocation[i], 0);
  alpha_strip.show();
}

static void allAlphaLightsOff() {
  for(uint16_t i=0; i<alpha_strip.numPixels(); i++) {
      alpha_strip.setPixelColor(i, 0); // turn off pixel
  }
  alpha_strip.show();
}

void updateTubeLights(float level) {
  uint16_t i, j;
  int brightness = (int)(255.0 * level);
  if (brightness > 255) {
    brightness = 255;
  }

  for (i = 0; i < tube_strip.numPixels(); i++) {
    tube_strip.setPixelColor(i, 255-brightness, brightness/2, 0);
  }
    
  tube_strip.show();
  delay(100);
}

static uint32_t encodeLetter(uint32_t c) {
  uint32_t lower = c;
  if (c >= 'A' && c <= 'Z') {
    lower = 'a' + (c - 'A');
  }
  
  if (lower < 'a' || lower > 'z') {
    return lower;
  }
  return codes[lower-'a'];
}
