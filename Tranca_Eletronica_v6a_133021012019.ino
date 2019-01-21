/*
#####################################################################################
# Arquivo:            Tranca_Eletronica_v6a133021012019.ino
# Micro-processador:  Arduino UNO         
# Linguagem:	        Wiring / C          
#						
#	Objetivo:           Tranca eletrônica para sistemas de segurança
#                     Door Lock System for Security Systems
#										  
#	Funcionamento:	    Ao pressionar a tecla #, libera para ser digitado a senha;
#                           se a senha estiver errada, volta ao estado inicial;
#                           Se a senha estiver correta, libera a porta ou sistema de segurança;
#                           Simulação através de LEDs, porém adaptável a relés, servo-motores e trancas elétricas
#                           Tons audíveis para orientação
#                           Display LCD 16x2 com mensagens de orientação
#                           Ao pressionar *, libera para ser digitado a senha antiga
#                           e em seguida libera para a entrada de uma nova senha
#                           que passa a ser a senha válida
#                           A nova senha fica armazenada na EEPROM interna do Arduino
#                           Um servo-motor foi adicionado para simular uma tranca eletrônica
#                           Todas as telas foram convertidas para o idioma Inglês bem como os comentários
#			
#   Autor:              Marcelo Moraes 
#   Data:               21/01/19	
#   Local:              Sorocaba - SP	
#					
#####################################################################################
  This code is a public domain example
 */
 
//Including libraries
#include <Wire.h>
#include <LiquidCrystal.h>
#include <Keypad.h>
#include <EEPROM.h>
#include <Servo.h>

//LCD object definitions
LiquidCrystal lcd(14, 15, 16, 17, 18, 19);
//declares LCD pins
//pins: R/W - 14, Enable - 15 and Data - 16, 17, 18 e 19
//used analogic pins 0, 1, 2, 3, 4, 5
//as digital pins 14, 15, 16, 17, 18, 19 respectively

//global and general purposes variables
int count = 0;
int duration = 200;
String StoredPassword = "1234";
String OldPassword = "0000";
String DefaultPassword = "0000";

//pins definitions
#define yellowPin 12
#define redPin    13
#define greenPin  11
#define audioPin  9
#define servoPin  10

//audibles tones frequency definitions in Hz
#define NOTE_B6  1976
#define NOTE_FS6 1480
#define NOTE_C6  1047
#define NOTE_AS5 932
#define NOTE_FS5 740


//keypad specific variables
const byte ROWS = 4; //four lines
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};//keypad layout definition mapping
byte rowPins[ROWS] = {5, 4, 3, 2}; //pins used for lines
byte colPins[COLS] = {6, 7, 8};    //pins used for columns

//keypad object definition
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

//servo motor definition
Servo doorLock;


 
void setup(){
  doorLock.attach(servoPin); //servo motor attaching
  lcd.begin(16, 2); //LCD mode initialization 16x2
  //lcd.backlight(); //when using LCD with backlight feature
  
  Serial.begin(9600); //Serial communication initialization
  
  //pins mode definition
  pinMode(audioPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(servoPin,OUTPUT);
  
  lcd.clear(); //clears LCD display
  
  init_screen(); //calls the initial screen

  //the line below need to be executed just one time or to reset entire system passwords
  //uncomment if necessary
  //Write_EEPROM_Password(DefaultPassword);

  StoredPassword =  Read_EEPROM_Password(); //reads the stored pasword from the EEPROM
  //doorLock.write(0); //door starts locked
}





void loop(){
  
  char key = keypad.getKey(); //take reads from the keypad
  if (key != NO_KEY){ //something was pressed in the keypad
    if (key == '#') { //it was pressed # key
      code_entry_screen(); //updates the display messages, emits sounds and turns on LED
      
      int entriesNumber = 0; //amount of entries made by the keypad
      String CurrentPassword;
      
      while (entriesNumber < 4 ){ //while not made 4 entries to complete the password digits
        char key = keypad.getKey(); //take reads from the keyboard
        
        if (key != NO_KEY){ //something was pressed in the keypad
          entriesNumber += 1; //increments the number of entries from the keypad
          CurrentPassword += key; //stores the keys into a string
          
          tone(audioPin, NOTE_C6, duration/2); //emits audible sounds for each entry made via keypad
          delay(duration);
          noTone(audioPin);
          
          if(entriesNumber == 4 ){ //reached 4 entries via keypad
            if(CurrentPassword == StoredPassword){ //checks if the current password is the same stores in the system EEPROM memory
              unlocked(); //if it is correct, access granted and unlock the door system
              init_screen(); //process restarts
              break; //breaks the entire loop
            }
            else{
              locked_screen();
              init_screen(); //process restarts
              break; //breaks the entire loop
            }
          }
          if(key == '#'){ //it was pressed # 
              init_screen(); //process restarts
              break; //breaks the entire loop
          }
        }
      }
    }

    
    if(key == '*'){ //it was pressed * from keypad it means change the password by a new one
      old_pass_screen(); //it is necessary to enter the currently password first
      
      int entriesNumber = 0;
      String CurrentPassword;
      
      while(entriesNumber < 4){
        char key = keypad.getKey();
        
        if(key != NO_KEY){
          entriesNumber += 1;
          CurrentPassword += key;
          
          tone(audioPin, NOTE_C6, duration/2);
          delay(duration);
          noTone(audioPin);
         
          if(entriesNumber == 4){
            if(CurrentPassword == StoredPassword){//if password is correct
              get_new_pass(); //calls hte function to change a password
              init_screen(); //the process restarts
              break; //breaks the entire loop
            }
            else{
              init_screen(); //the process restarts
              break; //breaks the entire loop
            }
          }
          if(key == '*'){ //if pressed * again without complete the password
            init_screen(); //the process restarts
            break; //breaks the entire loop
          }
        }
      }
    } 
  }
}



void locked_screen(){
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
    new_pass_screen(); //shows new password screen
    int entriesNumber = 0;
    String NewPassword;
    
    while(entriesNumber < 4){ 
      char key = keypad.getKey(); 
      if(key != NO_KEY){ 
        entriesNumber += 1; 
        NewPassword += key; 
        
        tone(audioPin, NOTE_C6, duration/2); 
        delay(duration);
        noTone(audioPin);
        
        if(entriesNumber == 4){
          StoredPassword = NewPassword;
          Write_EEPROM_Password(StoredPassword); //writes the new password into the arduino's EEPROM
          break;
        }
        if(key == '*'){
          break; // sai
        }
      }
    }
}



void new_pass_screen(){
  
  lcd.clear();
  lcd.print("  New Password");
  count = 0;
  lcd.setCursor(0,1);
  lcd.print("    Press *");
  
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
 
  lcd.clear();
  lcd.print(" Old Password?");
  count = 0;
  lcd.setCursor(0,1);
  lcd.print("    Press *");
  
  
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
  lcd.clear();
  lcd.print("      ABMS");
  lcd.setCursor(0,1);
  lcd.print("    TELECOM");
  delay(1000);
  lcd.clear();
  lcd.print("   Welcome...");
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
 
void code_entry_screen(){

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

  lcd.clear();
  lcd.print("   GARANTEED");
  lcd.setCursor(0,1);
  lcd.print("    ACCESS!");
  

  digitalWrite(redPin, LOW);
  digitalWrite(yellowPin, LOW);
  
  doorLock.write(180);
  
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
  
  doorLock.write(90); // Locks the door
}


String Read_EEPROM_Password(){
  String MyPassword;
  for (int x = 0; x < 4; x++) {
    char c = char(EEPROM.read(x));
   MyPassword += c;
  }
  delay(10);
  return MyPassword;
}


void Write_EEPROM_Password(String S){
  String MyPassword = S;
  for (int x = 0; x<4; x++){
 EEPROM.write(x, byte(MyPassword.charAt(x)));
 }
}
