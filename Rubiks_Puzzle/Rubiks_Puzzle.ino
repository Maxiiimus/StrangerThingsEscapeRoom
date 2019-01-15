#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_NeoPixel.h>

#define PIN    7
#define N_LEDS 21
int ledPin = 5;
int facePixel[] = {20, 16, 12, 8, 4, 0};

Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_LEDS, PIN, NEO_GRB + NEO_KHZ800);

#define RST_PIN   5     // Configurable, see typical pin layout above
#define SS_PIN    53   // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

/* Set your new UID here! */
#define  NEW_UID {0xDE, 0xAD, 0xBE, 0xEF}

// RFID first bytes for each side
#define  MASTER 0x2C
#define  RED 0x66
#define  ORANGE 0xE6
#define  YELLOW 0x16
#define  WHITE 0x36
#define  BLUE 0xB6
#define  GREEN 0x06
#define UNLOCKED 0b111111
#define CODE_LENGTH 6
static byte colorCodes[6] = {RED, ORANGE, YELLOW, GREEN, BLUE, WHITE};
uint32_t colors[6];

static byte code[CODE_LENGTH] = {RED, RED, YELLOW, BLUE, YELLOW, GREEN};
int pins[CODE_LENGTH] = {0b100000, 0b010000, 0b001000, 0b000100, 0b000010, 0b000001};
int currentAttempt = 0;
int combo = 0b000000;

MFRC522::MIFARE_Key key;

int solenoid = 12;
int redLed = 11;

void setup() {

  // initialize the digital pin as an output.
  pinMode(solenoid, OUTPUT);     
  //pinMode(greenLed, OUTPUT);
  pinMode(redLed, OUTPUT);
  digitalWrite(redLed, HIGH); 
  strip.begin();
  reset();

  colors[0] = strip.Color(255, 0, 0); // Red
  colors[1] = strip.Color(255, 100, 0); // orange
  colors[2] = strip.Color(127, 127, 0); // yellow
  colors[3] = strip.Color(0, 255, 0); // green
  colors[4] = strip.Color(0, 0, 255); // blue
  colors[5] = strip.Color(127, 127, 127); // white
  
  Serial.begin(9600);  // Initialize serial communications with the PC
  while (!Serial);     // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();         // Init SPI bus
  mfrc522.PCD_Init();  // Init MFRC522 card
  Serial.println(F("Ready to start decoding!"));
  
  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
}

// But of course this is a more proper approach
void loop() {
  
  // Look for new cards, and select one if present
  if ( ! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial() ) {
    delay(50);
    return;
  }

  // Now a card is selected. The UID and SAK is in mfrc522.uid.
  int scannedColor = mfrc522.uid.uidByte[0];
  
  // Dump UID
  Serial.print(F("Card UID First byte:"));
  Serial.println(scannedColor, HEX);

  if (currentAttempt >= CODE_LENGTH){
    Serial.print("Passed max CODE_LENGTH: ");
    Serial.println(currentAttempt);
    reset();
    return;
  }
  
  if (scannedColor == code[currentAttempt]) {
    combo = combo | pins[currentAttempt];
  }
  Serial.print("Lock combo state: ");
  Serial.println(combo, BIN);

  if (scannedColor == MASTER) {
    Serial.println("Found master! Unlocking.");
    unlock();
    reset();
    return;
  }

  bool validColor = false;
  for (int i = 0; i < CODE_LENGTH; i++)
  {
    if (scannedColor == colorCodes[i]) {
      Serial.print("Found color: ");
      Serial.println(i);
      strip.setPixelColor(facePixel[currentAttempt], colors[i]); 
      strip.show();
      validColor = true;
      break;
    }
  }
  if (!validColor) {
    lock();
    reset();
    return;
  }

  currentAttempt++;

  // Check if we've solved it
  if (currentAttempt == CODE_LENGTH) {
    if (combo == UNLOCKED) {
      Serial.println("Correct code entered. Unlocking.");
      unlock();
      //digitalWrite(greenLed, HIGH);   // turn the LED on (HIGH is the voltage level)
      //digitalWrite(solenoid, HIGH);   // turn the LED on (HIGH is the voltage level)
    } else {
      Serial.println("Incorrect code entered. Locking.");
      lock();
      reset();
      //digitalWrite(redLed, HIGH);   // turn the LED on (HIGH is the voltage level)
    }
    //reset();
  } else {
    delay(1000);
  }
}

static void lock() {
  int waitTime = 250;
  digitalWrite(solenoid, LOW);   // close lock
  for (int i = 0; i < 10; i++)
  {
     for (int j = 0; j < CODE_LENGTH; j++)
     {
       strip.setPixelColor(facePixel[j], colors[0]); // Turn all red
     }
     strip.show();
     delay(waitTime);
     for (int j = 0; j < CODE_LENGTH; j++)
     {
       strip.setPixelColor(facePixel[j], 0); // Turn all green
     }
     strip.show();
     delay(waitTime);
  }
}


static void unlock() {
  int waitTime = 1000;
  //delay(250);
  digitalWrite(solenoid, HIGH);   // open lock
  //delay(waitTime);
  for (int i = 0; i < 20; i++)
  {
     for (int j = 0; j < CODE_LENGTH; j++)
     {
       strip.setPixelColor(facePixel[j], colors[3]); // Turn all green
     }
     strip.show();
 
     delay(waitTime);
     waitTime = waitTime - 50;
     for (int j = 0; j < CODE_LENGTH; j++)
     {
       strip.setPixelColor(facePixel[j], 0); // Turn all green
     }
     strip.show();
     delay(waitTime);
  }
  digitalWrite(solenoid, LOW);   // close lock
}

static void reset() {
  combo = 0;
  currentAttempt = 0;
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, 0); // Erase pixel
  }
  strip.show();

  digitalWrite(solenoid, LOW);    // Close lock
  //digitalWrite(redLed, LOW);   // turn the LED on (HIGH is the voltage level)
  //digitalWrite(greenLed, LOW);   // turn the LED on (HIGH is the voltage level)

}
