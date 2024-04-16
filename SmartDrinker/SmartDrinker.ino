#define BLYNK_PRINT Serial

#define BLYNK_TEMPLATE_ID "TMPL2Svt59SpF"
#define BLYNK_TEMPLATE_NAME "Smart Drinker"
#define BLYNK_AUTH_TOKEN "h6J8JbFL4XmmryMYS2b7YDGFuHhWkQ_A"

// Select your modem:
#define TINY_GSM_MODEM_SIM800

#include <TinyGsmClient.h>
#include <BlynkSimpleTinyGSM.h>
#include <NewPing.h>

#define BAUD 9600

// #include <ThingSpeak.h>
// #define writeKey "SJ9T8CBVRVZP4XHQ"
// //char[] readKey = 'HCOYJEDWY9FITGLU'
// const long channelID = 2509533;
// const unsigned int drinkerVisitsField = 1;
// const unsigned int waterDepthField = 2;

#include <SoftwareSerial.h>
#include <PN532_SWHSU.h>
#include <PN532.h>

SoftwareSerial SWSerial(12,11); // RX, TX
PN532_SWHSU pn532swhsu( SWSerial );
PN532 nfc( pn532swhsu );
String tagId = "None", dispTag = "None";
byte nuidPICC[4];

// Your GPRS credentials
// Leave empty, if missing user or pass
char apn[]  = "internet";
char user[] = "";
char pass[] = "";

// Hardware Serial on Mega, Leonardo, Micro
// #define SerialAT Serial1

// or Software Serial on Uno, Nano

SoftwareSerial SerialAT(2, 3); // RX, TX

TinyGsm modem(SerialAT);


// Hook up HC-SR04 with Trig to Arduino Pin 9, Echo to Arduino pin 10
#define TRIGGER_PIN 7
#define ECHO_PIN 6
// #define RX_PIN 2 
// #define TX_PIN 1


#define MAX_DISTANCE 400	

// NewPing setup of pins and maximum distance.
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);


int measureDepth(){
  // Serial.print("Distance = ");
	int depth = sonar.ping_cm(MAX_DISTANCE);
	// Serial.println(" cm");
	// delay(500);
  return depth;
}

void SendData(int depth,  int times)
{
  if (isnan(depth)) {
    Serial.println(F("Failed to read from ultrasonic sensor!"));
    return;
  }

  // You can send any value at any time.
  Blynk.virtualWrite(V0, depth);
  Blynk.virtualWrite(V1, times);
  //-----------------------------------------------------------------------
}

void printDepth(){
  int depth = sonar.ping_cm(MAX_DISTANCE);
 Serial.println(depth);
}


void setup() {
	// Debug console
  Serial.begin(BAUD);

  delay(10);

   // Set GSM module baud rate
  SerialAT.begin(BAUD);
  delay(3000);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  Serial.println("Initializing modem...");
  modem.restart();

  // Unlock your SIM card with a PIN
  //modem.simUnlock("1234");

  Blynk.begin(BLYNK_AUTH_TOKEN, modem, apn, user, pass);
  Serial.println("hopefully we reach here");

  // initRFID();
}
int no_times = 8;
void loop() {

  int depth = measureDepth();
  // readNFC();
  

  Blynk.run();
  SendData(depth,no_times);

  delay(1000); // Send data every 5 seconds (adjust as needed)

}

void readNFC()
{
  boolean success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                       // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
  if (success)
  {
    Serial.print("UID Length: ");
    Serial.print(uidLength, DEC);
    Serial.println(" bytes");
    Serial.print("UID Value: ");
    for (uint8_t i = 0; i < uidLength; i++)
    {
      nuidPICC[i] = uid[i];
      Serial.print(" "); Serial.print(uid[i], DEC);
    }
    Serial.println();
    tagId = tagToString(nuidPICC);
    dispTag = tagId;
    Serial.print(F("tagId is : "));
    Serial.println(tagId);
    Serial.println("");
    no_times ++;
    // return tagId;
  }
  else
  {
    // PN532 probably timed out waiting for a card
    Serial.println("Timed out! Waiting for a card...");
    // return "";
  }
  delay(1000);  // 1 second halt
}

String tagToString(byte id[4])
{
  String tagId = "";
  for (byte i = 0; i < 4; i++)
  {
    if (i < 3) tagId += String(id[i]) + ".";
    else tagId += String(id[i]);
  }
  return tagId;
}

void initRFID(){
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
   while (! versiondata)
  {
    Serial.println("Didn't Find PN53x Module");
    delay(2000); // Halt
    versiondata = nfc.getFirmwareVersion();
  }
  // Got valid data, print it out!
  Serial.print("Found chip PN5");
  Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware ver. ");
  Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.'); 
  Serial.println((versiondata >> 8) & 0xFF, DEC);
  // Configure board to read RFID tags
  nfc.SAMConfig();
}