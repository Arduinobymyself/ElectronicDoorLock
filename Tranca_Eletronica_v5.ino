/*
#####################################################################################
#  	Arquivo:            Tranca_eletronica_v5.pde                                             
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
#                           Ao pressionar *, libera para ser digitado a senha antiga
#                           e em seguida para a entrada de uma nova senha
#                           que passa a ser a senha válida, enquanto o sistema está alimentado
#                           a senha volta para a default em caso de perda de energia
#
#                           Atualizado para LCD I2C e com possibilidade de colocar nova senha
#			
#   Autor:              Marcelo Moraes 
#   Data:               26/05/13	
#   Local:              Sorocaba - SP	
#					
#####################################################################################
 
  Este exemplo é um código de domínio público.
 */
 
// inclusão de bilibotecas
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

LiquidCrystal_I2C lcd(0x27,16,2);// declara LCD, endereço e tipo

// declarção de variáveis
int count = 0;

char pass [4] = {'1', '2', '3', '4'}; // senha padrão

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
};// deinição do layout do teclado
byte rowPins[ROWS] = {5, 4, 3, 2}; // pinagem para as linhas do teclado
byte colPins[COLS] = {6, 7, 8}; // pinagem para as colunas do teclado
// mapeamento do teclado
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
 
void setup(){
  lcd.init(); // inicializa o LCD
  lcd.backlight(); // com backlight
  Serial.begin(9600); // inicializa serial
  // modo dos pinos de audio e LEDs
  pinMode(audioPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  lcd.clear();// limpa LCD
  key_init();// inicializa processo
}
 
void loop(){
  char key = keypad.getKey(); // obtém informação do teclado
  if (key != NO_KEY){ // se foi teclado algo
    if (key == '#') {// se foi teclado #
      code_entry_init(); // mensagem, som e LED
      int entrada = 0; // variável de apoio; números de entradas feitas via teclado
      while (count < 4 ){ // enquanto não entrou os 4 números necessários para a senha
        char key = keypad.getKey(); // obtém informação do teclado
        if (key != NO_KEY){// se foi teclado algo
          entrada += 1; // aumenta contrador de entradas
          tone(audioPin, 1080, 100); // sinal audível
          delay(duration);
          noTone(audioPin);
          if (key == pass[count])count += 1; // verifica na sequencia da senha, se correto aumenta contador
          if ( count == 4 ){
            unlocked(); // chegou a 4 digitos corretos, libera acesso
          }
          if((key == '#') || (entrada == 4)){ // foi teclado # ou 4 entradas incorretas
            key_init();// inicializa 
            break;// interrompe loop
          }
        }
      }
    }
    if(key == '*'){ // se foi teclado *
      old_pass_check();// mensagem para entrar a senha antiga
      int entrada = 0;
      while (count < 4 ){
        char key = keypad.getKey();
        if (key != NO_KEY){
          entrada += 1;
          tone(audioPin, 1080, 100);
          delay(duration);
          noTone(audioPin);
          if (key == pass[count])count += 1;
          if ( count == 4 ){// foi teclado a senha antiga corretamente
            get_new_pass();// chama função para entrada da nova senha
          }
          if ((key == '*') || (entrada == 4)){// foi teclado * ou entrou 4 números errados
             key_init();// inicializa
            break; // interrompe loop
          }
        }
      }
    }  
  }
}

void locked(){
  lcd.clear();
  lcd.print("    ACESSO,");
  lcd.setCursor(0,1);
  lcd.print("    NEGADO!");
  for(int i=0; i<11; i++){
    tone(audioPin, 1080, 100);
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
    new_pass_entry(); // mensagem, som e LED
    int entrada = 0; // inicializa entrada
    while(count < 4){ // enquanto contrador for menor que 4
      char key = keypad.getKey(); // obtem informação do teclado
      if(key != NO_KEY){ // se algo foi teclado
        entrada += 1; // aumenta contador de entrada
        tone(audioPin, 1080, 100); // tom para cada dígito
        delay(duration);
        noTone(audioPin);
        pass[count] = key; // aramazena o novo dígito
        count += 1; // próximo dígito
        if(count == 4) break; // chegou a 4 digitos, interrompe loop     
        if((key == '*') || (entrada == 4)){// foi telcado * 4 entradas
          key_init();// inicializa sistema
          break; // sai
        }
      }
    }
}


void new_pass_entry(){
  // mensagem no display
  lcd.clear();
  lcd.print("   Nova Senha");
  count = 0;
  lcd.setCursor(0,1);
  lcd.print("    Tecle *");
  // gera sinal audível
  tone(audioPin, 1500, 100);
  delay(duration);
  noTone(audioPin);
  tone(audioPin, 1500, 100);
  delay(duration);
  noTone(audioPin);
  tone(audioPin, 1500, 100);
  delay(duration);
  noTone(audioPin);
  // somente LED amarelo aceso
  digitalWrite(redPin, LOW);
  digitalWrite(yellowPin, HIGH);
  digitalWrite(greenPin, LOW);
}

void old_pass_check(){
  // mensagem no display
  lcd.clear();
  lcd.print("  Senha antiga?");
  count = 0;
  lcd.setCursor(0,1);
  lcd.print("    Tecle *");
  // gera tom audível
  tone(audioPin, 1500, 100);
  delay(duration);
  noTone(audioPin);
  tone(audioPin, 1500, 100);
  delay(duration);
  noTone(audioPin);
  tone(audioPin, 1500, 100);
  delay(duration);
  noTone(audioPin);
  // somente LED amarelo aceso
  digitalWrite(redPin, LOW);
  digitalWrite(yellowPin, HIGH);
  digitalWrite(greenPin, LOW);
}
  
 
void key_init (){
  // mensagem no display
  lcd.clear();
  lcd.print("  Bem vindo...");
  lcd.setCursor(0,1);
  lcd.print("  Tecle: # ou *");
  count = 0;// contador para zero
  // somente LED vermelho aceso
  digitalWrite(redPin, HIGH);
  digitalWrite(yellowPin, LOW);
  digitalWrite(greenPin, LOW);
  // gera tom audível
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
  // mensagem no display
  lcd.clear();
  lcd.print(" Entre a Senha:");
  count = 0; // contador para zero
  // gera sinal audível
  tone(audioPin, 1500, 100);
  delay(duration);
  noTone(audioPin);
  tone(audioPin, 1500, 100);
  delay(duration);
  noTone(audioPin);
  tone(audioPin, 1500, 100);
  delay(duration);
  noTone(audioPin);
  // somente LED amarelo aceso
  digitalWrite(redPin, LOW);
  digitalWrite(yellowPin, HIGH);
  digitalWrite(greenPin, LOW);
}
 
void unlocked(){
  // mensagem no display
  lcd.clear();
  lcd.print("Acesso Liberado!");
  // somente LED verde aceso
  digitalWrite(redPin, LOW);
  digitalWrite(yellowPin, LOW);
  // pisca LED verde e gera sinal audível
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

