

/*  ___   ___  ___  _   _  ___   ___   ____ ___  ____  
 * / _ \ /___)/ _ \| | | |/ _ \ / _ \ / ___) _ \|    \ 
 *| |_| |___ | |_| | |_| | |_| | |_| ( (__| |_| | | | |
 * \___/(___/ \___/ \__  |\___/ \___(_)____)___/|_|_|_|
 *                  (____/    
 * www.osoyoo.com water drop sensor
 * program tutorial : http://osoyoo.com/2016/07/04/arduino-rfid/
 *  Copyright John Yu
 */


// libraries
#include <SPI.h>
#include <RFID.h>
#include <Servo.h>
#include "pitches.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// RFID definition
RFID rfid(10,5);

// 5 data bytes from cards
// you can read these bytes on the Serial console
byte userC[5] = {0xBA,0xB9,0x57,0x10,0x44}; // user C
byte userB[5] = {0xE5,0x60,0xD8,0x65,0x38}; // User B
byte userA[5] = {0x01,0x9A,0x92,0x2b,0x23}; // User A
// Add allowed card IDs here

// // LCD address and type declaration
LiquidCrystal_I2C lcd(0x3F,20,4);

byte serNum[5];
byte data[5];
int cardRead, light;  // card read 1 = good 0 = bad for playTune function
String Name;   // used for user name displayMsg function

// Melodies definition: access, welcome and rejection
int access_melody[] = {NOTE_G6,0, NOTE_A4,0, NOTE_B4,0, NOTE_A4,0, NOTE_B4,0, NOTE_C5,0};
int access_noteDurations[] = {2,8,8,8,8,4,8,8,8,8,8,4};
int fail_melody[] = {NOTE_F6,0, NOTE_F6,0, NOTE_F6,0,NOTE_F6,0, NOTE_F6,0, NOTE_F6,0,NOTE_F6,0, NOTE_F6,0, NOTE_F6,0};
int fail_noteDurations[] = {8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8};
int all_clear[] = {NOTE_CS7,0, NOTE_FS7,0};
int all_clear_durations[] = {10,36,12,16};

// pins definition - LED, Buzzer and Servo-motor
const int pResistor = A0;
int LED_access =2;
int LED_intruder =3;
int speaker_pin = 8;
int servoPin = 9;

// servo motor definition
Servo doorLock;


void setup(){
  doorLock.attach(servoPin);    // servo motor attaching
  Serial.begin(9600);           // Serial communication initialization
  lcd.init();                   // LCD initialization
  lcd.backlight();
  lcd.clear();                  // Clears the LCD display
  SPI.begin();                  // SPI communication initialization
  rfid.init();                  // RFID module initialization
  Serial.println("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  Serial.println("+ XXXXXXX Security System                                                             +"); 
  Serial.println("+ SigmaHacks Entry 2019                                                               +");
  Serial.println("+ Ethan Cho and Sidarth Mamidanna                                                     +");
  Serial.println("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  displayMsgInitial();
  //delay(3000);
  pinMode(LED_access,OUTPUT);
  pinMode(LED_intruder,OUTPUT);
  pinMode(speaker_pin,OUTPUT);
  pinMode(servoPin,OUTPUT);
  pinMode(pResistor, INPUT);
}
 
void loop(){
  //lcd.noBacklight();
  // Here you can create a variable for each user
  // NAME_card ou KEY_card
  boolean userC_card = true; // that is user C
  boolean userB_card = true; // that is user B
  boolean userA_card = true; // that is user A
  //put another users here
  
  if (rfid.isCard()){ // valid card found
    if (rfid.readCardSerial()){ // reads the card
      //delay(1000);
      data[0] = rfid.serNum[0]; // stores the serial number
      data[1] = rfid.serNum[1];
      data[2] = rfid.serNum[2];
      data[3] = rfid.serNum[3];
      data[4] = rfid.serNum[4];
    }
  rfid.halt(); // RFID module to standby
  cardRead = 0;
  
  //displayID_LCD();
  displayID_Console(); // used for testing purposes only. Prints the Card ID at the serial console 
  for(int i=0; i<5; i++){
    if(data[i] != userC[i]) userC_card = false; // if it is not a valid card, put false to this user
    if (data[i] != userB[i]) userB_card = false;
    if (data[i] != userA[i]) userA_card = false;
    // Here you can check the another allowed users, just put lines like above with the user name
  }
  
  Serial.println(); // used for testing purposes  only
  if (servoPin = 10){ // if a valid card was found
    cardRead = 1;
    playTune(cardRead);
    displayID_LCD();
    //playTune(cardRead);
    displayMsgOK("C");
    scan();
    //playTune(cardRead);
    //lcd.print("Scan your ID on the sensor below");
  }
  else if(userB_card){
    cardRead = 1;
    displayMsgOK("B");
    playTune(cardRead);
  }
  else if(userA_card){
    cardRead = 1;
    displayMsgOK("A");
    playTune(cardRead);
  }
  // another cards analysis, put many blocks like this 
  // as many users you have
  
  else{ // if a card is not recognized
    displayMsgNOK();
    Serial.println("Card not recognized!  contact administration!"); // used for testing purposes  only
    digitalWrite(LED_intruder, HIGH); // turn on the red LED
    playTune(cardRead);
    delay(1000);
    digitalWrite(LED_intruder, LOW); // turn off the red LED
  }
  if (userC_card || userB_card || userA_card){// add another user using an logical or condition || 
    Serial.println("Access Granted!... Welcome to Osoyoo.com!"); // used for testing purposes  only
    digitalWrite(LED_access,HIGH); // turn on the green LED
    doorLock.write(0); // releases the door, you need to adjust this to positioning the servo according your door locker
    delay(3000); // wait 5 senconds 
    doorLock.write(180); // Locks the door, brings the serve to the original position should be adjusted too....
    
    digitalWrite(LED_access,LOW); // turn off the green LED
  }
  Serial.println(); // used for testing purposes  only
  delay(2000);
  displayMsgInitial();
  rfid.halt(); // wait till sense a card over the reader
  //lcd.noBacklight(); // turn the LCD backlight off
  
  }
}


void scan(){
  delay(5000);
  light = analogRead(pResistor);
  
  //You can change value "25"
  Serial.println(light);
  if (light > 330){
    threatmsg();  //Turn led off
    playTune(0);
    delay(1000000);
    
  }
  else{
    displayAllClear(); //Turn led on
    playTune(2);
  }
}

//========== Function to play the access granted or denied tunes ==========
void playTune(int Scan) {    
  if (Scan == 1) // A Good card Read
  { 
    for (int i = 0; i < 2; i++)    //loop through the notes
        { // Good card read
          int access_noteDuration = 1000 / access_noteDurations[i];
          tone(speaker_pin, access_melody[i], access_noteDuration);
          int access_pauseBetweenNotes = access_noteDuration * 1.30;
          delay(access_pauseBetweenNotes);
          noTone(speaker_pin);
       }
  }     
  else if (Scan == 2) // A Good card Read
  { 
    for (int i = 0; i < 4; i++)    //loop through the notes
        { // Good card read
          int clear_noteDuration = 1000 /  all_clear_durations[i];
          tone(speaker_pin, all_clear[i], clear_noteDuration);
          int clear_pauseBetweenNotes = clear_noteDuration * 1.30;
          delay(clear_pauseBetweenNotes);
          noTone(speaker_pin);
       }
  }  
    else // A Bad card read
       for (int i = 0; i < 16; i++)    //loop through the notes 
       { 
          int fail_noteDuration = 1000 / fail_noteDurations[i];
          tone(speaker_pin, fail_melody[i], fail_noteDuration);
          int fail_pauseBetweenNotes = fail_noteDuration * 1.30;
          delay(fail_pauseBetweenNotes);
          noTone(speaker_pin);
       }
  }
  
  
//========== Display LCD messages for the users ========== 
void displayMsgOK(String user)  
{
    //display：" Welcome user"
    lcd.clear();
    lcd.backlight();
    lcd.setCursor(0,0);
    lcd.print("    Scanning");
    lcd.setCursor(0,1);
    lcd.print("your belongings");
    
} 
//void
void displayMsgInitial()  
{
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Empty pockets  ");
    lcd.setCursor(0,1);
    lcd.print("then scan ID  ");
}
void displayAllClear()
{
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("***ALL CLEAR***");
    lcd.setCursor(0,1);
    lcd.print("You may proceed");
}
void threatmsg(){
    Serial.println("THREAT detected in lane 1");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("THREAT DETECTED");
    lcd.setCursor(0,1);
    lcd.print("   Please wait  ");
}
void displayID_LCD(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.backlight();
  lcd.printstr("ID is Valid");
  //display："* ID Founf *"
  /*lcd.setCursor(0,1);
  lcd.print("*");
  lcd.setCursor(1,1);
  lcd.print(" ");
  lcd.setCursor(2,1);
  lcd.print("I");
  lcd.setCursor(3,1);
  lcd.print("D");
  lcd.setCursor(4,1);
  lcd.print(" ");
  lcd.setCursor(5,1);
  lcd.print("F");
  lcd.setCursor(6,1);
  lcd.print("o");
  lcd.setCursor(7,1);
  lcd.print("u");
  lcd.setCursor(8,1);
  lcd.print("n");
  lcd.setCursor(9,1);
  lcd.print("d");
  lcd.setCursor(10,1);
  lcd.print("  ");
  lcd.setCursor(11,1);
  lcd.print("*");*/
  
  //Serial.print("Card found - code:");
  lcd.setCursor(0,2);
  lcd.print("*    ");
  if(rfid.serNum[0] < 16){
    lcd.print("0");
  }
  lcd.print(rfid.serNum[0],HEX);
  
  if(rfid.serNum[1] < 16){
    lcd.print("0");
  }
  lcd.print(rfid.serNum[1],HEX);
  
  if(rfid.serNum[2] < 16){
    lcd.print("0");
  }
  lcd.print(rfid.serNum[2],HEX);
  
  if(rfid.serNum[3] < 16){
    lcd.print("0");
  }
  lcd.print(rfid.serNum[3],HEX);
  
  if(rfid.serNum[4] < 16){
    lcd.print("0");
  }
  lcd.print(rfid.serNum[4],HEX);
  lcd.print("    *");
  
  lcd.setCursor(0,3);
  lcd.print("********************");
  delay(1000); // shows the ID for about 1000ms
  lcd.clear();
  lcd.noBacklight();
}



void displayID_Console(){
  Serial.print("Card found - code:");
  if(rfid.serNum[0] < 16){
    Serial.print("0");
  }
  Serial.print(rfid.serNum[0],HEX);
  Serial.print(" ");
  if(rfid.serNum[1] < 16){
    Serial.print("0");
  }
  Serial.print(rfid.serNum[1],HEX);
   Serial.print(" ");
  if(rfid.serNum[2] < 16){
    Serial.print("0");
  }
  Serial.print(rfid.serNum[2],HEX);
   Serial.print(" ");
  if(rfid.serNum[3] < 16){
    Serial.print("0");
  }
  Serial.print(rfid.serNum[3],HEX);
   Serial.print(" ");
  if(rfid.serNum[4] < 16){
    Serial.print("0");
  }
  Serial.print(rfid.serNum[4],HEX);
   Serial.print(" ");
}
