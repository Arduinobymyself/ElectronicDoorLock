/*
#####################################################################################
#  	Arquivo:            Tranca_eletronica_v2.pde                                             
#       Micro-processador:  Arduino UNO         
#  	Linguagem:	    Wiring / C          
#						
#	Objetivo:           Tranca Eletrônica
#										  
#	Funcionamento:	    Ao pressionar #, libera para ser digitado a senha;
#                           se a senha estiver errada, volta ao estado inicial;
#                           Se a senha estiver correta, libera a porta;
#                           Simulação através de LEDs, porém adaptável a relés e trancas
#                           Tons audíveis para orientação
#                           Display com as mensagens
#			
#   Autor:              Marcelo Moraes 
#   Data:               20/02/12	
#   Local:              Sorocaba - SP	
#					
#####################################################################################
 
  Este exemplo é um código de domínio público.
 */
 
 
#include <Wire.h>

#include <LiquidCrystal_I2C.h>

#include <Keypad.h>

LiquidCrystal_I2C lcd(0x27,16,2);

int count = 0;
char pass [4] = {'1', '9', '7', '1'};
const int yellowPin = 11;
const int redPin = 12;
const int greenPin = 10;
const int audioPin = 9;
const int duration = 200;
const byte ROWS = 4; //Quatro linhas
const byte COLS = 3; //Três colunas
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {5, 4, 3, 2}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {6, 7, 8}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
 
void setup(){
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  Serial.begin(9600);
  
  pinMode(audioPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  lcd.clear();
  key_init();
}
 
void loop(){
  char key = keypad.getKey();
  if (key != NO_KEY){
    if (key == '#') {
      code_entry_init();
      int entrada = 0;
      while (count < 4 ){
        char key = keypad.getKey();
        if (key != NO_KEY){
          entrada += 1;
          tone(audioPin, 1080, 100);
          delay(duration);
          noTone(audioPin);
          if (key == pass[count])count += 1;
          if ( count == 4 ) unlocked();
          if ((key == '#') || (entrada == 4)){
             key_init();
            break;
          }
        }
      }
    }
  }
}
 
void key_init (){
  lcd.clear();
  lcd.print("Aguardando...");
  lcd.setCursor(0,1);
  lcd.print("Tecle #");
  
  count = 0;
  digitalWrite(redPin, HIGH);
  digitalWrite(yellowPin, LOW);
  digitalWrite(greenPin, LOW);
  tone(audioPin, 1080, 100);
  delay(duration);
  noTone(audioPin);
  tone(audioPin, 980, 100);
  delay(duration);
  noTone(audioPin);
  tone(audioPin, 770, 100);
  delay(duration);
  noTone(audioPin);
}
 
void code_entry_init(){
  lcd.clear();
  lcd.print("Entre a Senha:");
  
  count = 0;
  tone(audioPin, 1500, 100);
  delay(duration);
  noTone(audioPin);
  tone(audioPin, 1500, 100);
  delay(duration);
  noTone(audioPin);
  tone(audioPin, 1500, 100);
  delay(duration);
  noTone(audioPin);
  digitalWrite(redPin, LOW);
  digitalWrite(yellowPin, HIGH);
  digitalWrite(greenPin, LOW);
}
 
void unlocked(){
  lcd.clear();
  lcd.print("Acesso Liberado!");
  
  digitalWrite(redPin, LOW);
  digitalWrite(yellowPin, LOW);
  // while ( true ){
    for (int x = 0; x < 5; x++){ 
    digitalWrite(greenPin, HIGH);
    tone(audioPin, 2000, 100);
    delay(duration);
    noTone(audioPin);
    digitalWrite(greenPin, LOW);
    tone(audioPin, 2000, 100);
    delay(duration);
    noTone(audioPin);
    delay(250);
  }
}

