/*
#####################################################################################
# Arquivo:            Tranca_Eletronica_v6c141301022019.ino
# Micro-processador:  Arduino UNO         
# Linguagem:	        Wiring / C          
#						
#	Objetivo:           Electronic Door Lock for Security Systems
#                     
#										  
#	Funcionamento:	    Pressing the # key you can enter the system password;
#                       If the password is wrong, returns to the initial state;
#                       If the password is correct, the security system is released;
#                       Messages, audible tones and LEDs are used to shows the system status and for orientation;
#                       The messages are displayed in a LCD display;
#                       A normal 16x2 LCD communication or a 20x4 I2C communication could be used;
#                     Pressing the * key you can enter a new system password;
#                       The new password is stored in the Arduino's internal EEPROM memory;
#                     A Servo-Motor or a Relay could be used to Lock/Unlock the system;
#                     The system uses a 4x4 keypad layout;
#                     Password could have numbers and letters;
#                     A Real Time Clock RTC is used to shows the current Time at the Display
#                     A timer was implemented in order to go back to the first screen 
#                       if a person does not type any key 
#                       (the person has 5 seconds to entry the correct password)
#			
# Autor:              Marcelo Moraes 
# Data:               01/02/19	
# Local:              Sorocaba - SP	
#					
#####################################################################################
  This code is a public domain example
 */
 
//Including libraries
#include <Wire.h>
//#include <LiquidCrystal.h>    //IF YOU ARE USING A NORMAL LCD DISPLAY UNCOMMENT THIS LINE
#include <LiquidCrystal_I2C.h>  //IF YOU ARE USING AN I2C LCD DISPLAY UNCOMMENT THIS LINE
#include <Keypad.h>
#include <EEPROM.h>
//#include <Servo.h>              //IF YOU ARE USING A SERVO-MOTOR UNCOMMENT THIS LINE
#include "RTClib.h"             //Real Time Clock

//LCD object definitions

//IF YOU ARE USING A NORMAL LCD DISPLAY UNCOMMENT THE LINE BELOW
//LiquidCrystal lcd(14, 15, 16, 17, 18, 19);
//declares LCD pins
//pins: R/W - 14, Enable - 15 and Data - 16, 17, 18 e 19
//used analogic pins 0, 1, 2, 3, 4, 5
//as digital pins 14, 15, 16, 17, 18, 19 respectively

//IF YOU ARE USING AN I2C LCD DISPLAY UNCOMMENT THE LINE BELOW
LiquidCrystal_I2C lcd(0x27,16,2);
//declares LCD I2C address and format
//SDA must be conneted to Arduino's A4 pin
//SCL must be conneted to Arduino's A5 pin

//RTC object definition
RTC_DS1307 RTC;

//global and general purposes variables
int lcd_rows = 2;
int lcd_cols = 16;
int duration = 200;
unsigned long startMillis;
unsigned long currentMillis;
//unsigned long previousMillis = 0;
unsigned long period = 5000;

//adjust password length value to use a diffent password length
//remember to adjust all other password values for the system
int PasswordLength = 4;
int address = 0;
String StoredPassword = "0000";
String OldPassword = "0000";
String DefaultPassword = "0000";


//pins definitions
#define yellowPin 12
#define redPin    11
#define greenPin  13
#define audioPin  9
//#define servoPin  10 //UNCOMMENT THIS LINE WHEN USING SERVO-MOTOR AS LOCK/UNLOCK METHOD
#define relayPin  10 //UNCOMMENT THIS LINE WHEN USING RELAY AS LOCK/UNLOCK METHOD

//audibles tones (musical notes frequencies) definitions in Hz 
#define NOTE_B6  1976
#define NOTE_FS6 1480
#define NOTE_C6  1047
#define NOTE_AS5 932
#define NOTE_FS5 740


//keypad specific variables
const byte ROWS = 4;   //four lines
const byte COLS = 4;   //four columns
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};//keypad layout definition mapping
byte rowPins[ROWS] = {0, 1, 2, 3};    //pins used for lines
byte colPins[COLS] = {4, 5, 6, 7};    //pins used for columns

//keypad object definition
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

//servo motor definition
//Servo doorLock; //UNCOMMENT THIS LINE WHEN USING SERVO-MOTOR AS LOCK/UNLOCK METHOD



//functions prototypes and some comments
void locked_screen();                         //just shows the system locked messages, sounds and LEDs when an incorrect password is typed
void get_new_pass();                          //used to get the new password entries
void new_pass_screen();                       //just shows the get new password messages, sounds and LEDs when a new password is requested
void old_pass_screen();                       //just shows the old password messages, sounds and LEDs when a old password is requested
void init_screen();                           //just shows the initial screen messages, sounds and LEDs
void pass_entry_screen();                     //just shows the password entry screen messages, sounds and LEDs when requested
void unlocked();                              //just shows the unlocked screen messages, sounds and LEDs when a correct password is typed, also performs the door lock/unlock actions
String Read_EEPROM_Password();                //reads a password stored in the Arduino's EEPROM
void Write_EEPROM_Password(String S);         //writes a password into the Arduino's EEPROM
void Mask_Password_Digits(int i);             //put * chars when typing a password (just a special effect)
void clearEEPROM();                           //clears the EEPROM memory
String fixZero(int zero);                     //
String rightTime();                           //shows the System Real Time Clock


 
void setup(){
  RTC.begin(); //RTC module initialization
  //doorLock.attach(servoPin); //servo-motor attaching, UNCOMENT THIS LINE WHEN USING SERVO-MOTOR AS LOCK/UNLOCK METHOD
  lcd.init(); //UNCOMMENT THIS LINE WHEN USING AN I2C LCD DISPLAY
  //lcd.begin(16, 2); //UNCOMMENT THIS LINE WHEN USING A NORMAL 16X2 LCD DISPLAY
  lcd.backlight(); //when using LCD with backlight feature
  
  //Serial.begin(9600); //Serial communication initialization; must be used only for debug purposes
  
  //pins mode definition
  pinMode(audioPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  //pinMode(servoPin,OUTPUT); //UNCOMENT THIS LINE WHEN USING SERVO-MOTOR AS LOCK/UNLOCK METHOD
  pinMode(relayPin,OUTPUT); //UNCOMENT THIS LINE WHEN USING RELAY AS LOCK/UNLOCK METHOD
  
  lcd.clear(); //clears LCD display
  init_screen(); //calls the initial screen function

  //at very first time using thie system you must to store in the Arduino's EEPROM a default password
  //the lines below need to be executed just one time; or for the entire system password reset
  //clearEEPROM();
  //Write_EEPROM_Password(DefaultPassword); //UNCOMMENT THIS LINE JUST WHEN THE SYSTEM PASSWORD RESET IS NEEDED
  
  StoredPassword =  Read_EEPROM_Password(); //reads the stored pasword from the arduino's EEPROM

  //checks if the RTC is running, if not adjust dateand time to the compilation time.
  if (!RTC.isrunning()) {
    //Serial.println("RTC stoped, I'll adjust time and date with compilation time...");
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
}





void loop(){
  //DateTime Now = RTC.now(); //gets date and time
  char key = keypad.getKey(); //take reads from the keypad
 
  if (key != NO_KEY){ //something was pressed in the keypad   
    if (key == '#') { //it was pressed # key
      pass_entry_screen(); //updates the display messages, emits sounds and turns on LED
      
      int entriesNumber = 0; //amount of entries made by the keypad
      String CurrentPassword;

       startMillis = millis();  //initial start time
      
      while (entriesNumber < PasswordLength ){ //while not made PasswordLength entries to complete the password digits
        char key = keypad.getKey(); //take reads from the keyboard
        
        if (key != NO_KEY){ //something was pressed in the keypad
          Mask_Password_Digits(entriesNumber); //mask currently password numbers format as *
          entriesNumber += 1; //increments the number of entries from the keypad
          CurrentPassword += key; //stores the keys into a string
          
          tone(audioPin, NOTE_C6, duration/2); //emits audible sounds for each entry made via keypad
          delay(duration); //pause between notes
          noTone(audioPin); //stops emitting sound
          
          if(entriesNumber == PasswordLength ){ //reached the password total length entries via keypad
            if(CurrentPassword == StoredPassword){ //checks if the current typed password is the same as stored in the system EEPROM memory
              unlocked(); //if it is correct, the access is garanteed and just unlock the door system
              init_screen(); //process restarts
              break; //breaks the entire loop
            }
            else{
              locked_screen(); //if the password typed is not correct, the access is denied
              init_screen(); //process restarts
              break; //breaks the entire loop
            }
          }
          if(key == '#'){ //the password typed is incomplete and or it was pressed #, so get out
              init_screen(); //process restarts
              break; //breaks the entire loop
          }
        }
        currentMillis = millis();  //get the current "time" (actually the number of milliseconds since the program started)
        if (currentMillis - startMillis >= period){  //test whether the period has elapsed
          init_screen(); //process restarts
          startMillis = currentMillis;  //IMPORTANT to save the start time of the current LED state.
          break; //breaks the entire loop
        }
      }
    }

    
    if(key == '*'){ //it was pressed * from keypad it means change the password by a new one
      old_pass_screen(); //it is necessary to enter the currently or old password first
      
      int entriesNumber = 0;
      String CurrentPassword;

      startMillis = millis();  //initial start time
      
      while(entriesNumber < PasswordLength){
        char key = keypad.getKey();
        
        if(key != NO_KEY){
          Mask_Password_Digits(entriesNumber);
          entriesNumber += 1;
          CurrentPassword += key;
          
          tone(audioPin, NOTE_C6, duration/2);
          delay(duration);
          noTone(audioPin);
         
          if(entriesNumber == PasswordLength){
            if(CurrentPassword == StoredPassword){ //if password is correct
              get_new_pass(); //calls hte function to change the password
              init_screen(); //the process restarts
              break; //breaks the entire loop
            }
            else{
              init_screen(); //the process restarts
              break; //breaks the entire loop
            }
          }
          if(key == '*'){ //the password typed is incomplet and it was pressed * so get out
            init_screen(); //the process restarts
            break; //breaks the entire loop
          }
        }
        currentMillis = millis();  //get the current "time" (actually the number of milliseconds since the program started)
        if (currentMillis - startMillis >= period){  //test whether the period has elapsed
          init_screen(); //process restarts
          startMillis = currentMillis;  //IMPORTANT to save the start time of the current LED state.
          break; //breaks the entire loop
        }
      }
    } 
  }


  /*
  currentMillis = millis();  //get the current "time" (actually the number of milliseconds since the program started)
  if (currentMillis - previousMillis >= period)  //test whether the period has elapsed
  {
    lcd.noBacklight();  //if so, disable LCD Back light
    previousMillis = millis();  //IMPORTANT to save the start time
  }
  */

   lcd.setCursor(0,0);
   lcd.print("   "+rightTime());
}




//FUNCTIONS.........................................................................

void locked_screen(){
  //show the "locked" screen
  lcd.clear();
  lcd.print("     ACCESS");
  lcd.setCursor(0,1);
  lcd.print("     DENIED!");
  
  for(int i=0; i<11; i++){
    tone(audioPin, NOTE_C6, duration/2);
    digitalWrite(yellowPin,LOW);
    digitalWrite(redPin,HIGH);
    delay(duration);
    noTone(audioPin);
    digitalWrite(redPin,LOW);
    delay(100);
  }
  delay(1000);
}

void get_new_pass(){
    new_pass_screen(); //shows the "new password" screen
    int entriesNumber = 0;
    String NewPassword;
    
    while(entriesNumber < PasswordLength){ 
      char key = keypad.getKey(); 
      if(key != NO_KEY){ 
        Mask_Password_Digits(entriesNumber);
        entriesNumber += 1; 
        NewPassword += key; 
        
        tone(audioPin, NOTE_C6, duration/2); 
        delay(duration);
        noTone(audioPin);
        
        if(entriesNumber == PasswordLength){
          StoredPassword = NewPassword; //make the stored password as the new password
          Write_EEPROM_Password(StoredPassword); //writes the new password into the arduino's EEPROM
          break;
        }
        if(key == '*'){
          break;
        }
      }
    }
}

void new_pass_screen(){
  //shows the "new password" screen
  lcd.clear();
  lcd.print("  New Password");
  
  tone(audioPin, NOTE_FS6, duration/2);
  delay(duration);
  noTone(audioPin);
  tone(audioPin, NOTE_FS6, duration/2);
  delay(duration);
  noTone(audioPin);
  tone(audioPin, NOTE_FS6, duration/2);
  delay(duration);
  noTone(audioPin);
 
  digitalWrite(redPin, LOW);
  digitalWrite(yellowPin, HIGH);
  digitalWrite(greenPin, LOW);
}

void old_pass_screen(){
  //shows the "old password" screen
  lcd.clear();
  lcd.print(" Old Password?");
  
  tone(audioPin, NOTE_FS6, duration/2);
  delay(duration);
  noTone(audioPin);
  tone(audioPin, NOTE_FS6, duration/2);
  delay(duration);
  noTone(audioPin);
  tone(audioPin, NOTE_FS6, duration/2);
  delay(duration);

  noTone(audioPin);
  
  digitalWrite(redPin, LOW);
  digitalWrite(yellowPin, HIGH);
  digitalWrite(greenPin, LOW);
}

void init_screen(){
  //show the "initial" screen
  lcd.clear();
  lcd.print("      ABMS");
  lcd.setCursor(0,1);
  lcd.print("    TELECOM");
  delay(1000);
  lcd.clear();
  //lcd.print("   Welcome...");
  lcd.setCursor(0,1);
  lcd.print("  Press # or *");

  digitalWrite(redPin, HIGH);
  digitalWrite(yellowPin, LOW);
  digitalWrite(greenPin, LOW);
  
  tone(audioPin, NOTE_C6, duration/2);
  delay(duration);
  noTone(audioPin);
  tone(audioPin, NOTE_AS5, duration/2); 
  delay(duration);
  noTone(audioPin);
  tone(audioPin, NOTE_FS5, duration/2);
  delay(duration);
  
  noTone(audioPin);

}

void pass_entry_screen(){
  //shows the "password entry" screen
  lcd.clear();
  lcd.print("   Password?");

  tone(audioPin, NOTE_FS6, duration/2);
  delay(duration);
  noTone(audioPin);
  tone(audioPin, NOTE_FS6, duration/2);
  delay(duration);
  noTone(audioPin);
  tone(audioPin, NOTE_FS6, duration/2);
  delay(duration);
  
  noTone(audioPin);
  
  digitalWrite(redPin, LOW);
  digitalWrite(yellowPin, HIGH);
  digitalWrite(greenPin, LOW);
}

void unlocked(){
  //show the "unlock" screen
  lcd.clear();
  lcd.print("   GARANTEED");
  lcd.setCursor(0,1);
  lcd.print("    ACCESS!");
  

  digitalWrite(redPin, LOW);
  digitalWrite(yellowPin, LOW);

  //unlock method
  //doorLock.write(180); //UNCOMMENT THIS LINE WHEN USING SERVO-MOTOR AS LOCK/UNLOCK METHOD
  digitalWrite(relayPin,HIGH); //UNCOMMENT THIS LINE WHEN USING RELAY AS LOCK/UNLOCK METHOD
  
  for (int x = 0; x < 5; x++){ 
    digitalWrite(greenPin, HIGH);
    tone(audioPin, NOTE_B6, duration/2);
    delay(duration);
    noTone(audioPin);
    digitalWrite(greenPin, LOW);
    tone(audioPin, NOTE_B6, duration/2);
    delay(duration);
    
    noTone(audioPin);
    
    delay(250);
  }

  //lock method
  //doorLock.write(90); //UNCOMMENT THIS LINE WHEN USING SERVO-MOTOR AS LOCK/UNLOCK METHOD
  digitalWrite(relayPin,LOW); //UNCOMMENT THIS LINE WHEN USING RELAY AS LOCK/UNLOCK METHOD
}

String Read_EEPROM_Password(){
  //reads the password from the arduino's EEPROM
  String MyPassword;
  //interctive loop that reads bytes according the password length from the memory
  for (int x = 0; x < PasswordLength; x++) {
    char c = char(EEPROM.read(x));
   MyPassword += c;
  }
  delay(10);
  return MyPassword;
}

void Write_EEPROM_Password(String S){
  //writes the password to the arduino's EEPROM
  String MyPassword = S;
  //interctive loop that writes bytes according the password length to the memory
  for (int x = 0; x<PasswordLength; x++){ 
 EEPROM.write(x, byte(MyPassword.charAt(x)));
 }
}

void Mask_Password_Digits(int i){
  //just fills LCD positions with character "*"
  lcd.setCursor(((lcd_cols-PasswordLength)/2)+i,1);
  lcd.print("*");
}

void clearEEPROM(){
  //Serial.print(EEPROM.length());
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    if(EEPROM.read(i) != 0) //skip already "empty" addresses
    {
      EEPROM.write(i, 0); //write 0 to address i
    }
  }
  //Serial.println("EEPROM erased");
  address = 0;  //reset address counter
}

String rightTime(){
  DateTime Now = RTC.now();//reads date and time values
  
  String clock_date = " ";
  String clock_time = " ";
  
  int _hour = Now.hour();
  int _minute = Now.minute();
  int _second = Now.second();
  clock_time += fixZero(_hour);
  clock_time += ":";
  clock_time += fixZero(_minute);
  clock_time += ":";
  clock_time += fixZero(_second);

  int _day = Now.day();  
  int _month = Now.month();
  int _year = Now.year();
  clock_date += fixZero(_day);
  clock_date += "/";
  clock_date += fixZero(_month);
  clock_date += "/";
  clock_date += _year;
  return clock_time;
}


//this function fix the numeral to 2 digits when the integer have only one digit
String fixZero(int zero){
  String fixed;
  if (zero < 10) fixed += "0";
  fixed += zero;
  return fixed;
}
