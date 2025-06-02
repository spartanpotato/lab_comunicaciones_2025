/*
ver ::cl 20120520
Configuracion basica para modulo receptor  RR 10
Utiliza libreria VirtualWire.h
pin 01 5v
pin 02 Tierra
pin 03 antena externa
pin 07 tierra
pin 10 5v
pin 11 tierra
pin 12 5v
pin 14 Arduino pin digital 2
pin 15 5v
*/

#include <VirtualWire.h>

int verde = 4;
int const MESSAGE_TAM = 3; //Bytes
int const ID = 2;
int const ID_RECEPTOR = 2;
int const TAM = 7;

String image_total = "";


// Seq  ID  ID_Receptor Message checksum
// []   []      []      [][][]     []

String get_byte(uint8_t message){
  String bits = "";
  for(int i = 7; i>=0; i--){
    uint8_t bit =(message>>i)&1;
    bits+= String(bit);
  }
  return bits;
}

String get_message(uint8_t* message){
  String checksum = get_byte(message[TAM-1]);
  String image = "";
  for(int i = 3; i<=TAM-1; i++){
    image+= get_byte(message[i]);
  }
  return image;
}

void setup(){
    Serial.begin(9600);
    Serial.println("Configurando Recepcion");
    vw_set_ptt_inverted(true); 
    vw_setup(2000);
    vw_set_rx_pin(2);
    vw_rx_start();

    pinMode(verde, OUTPUT);
}

void loop(){
    uint8_t buf[VW_MAX_MESSAGE_LEN];

    uint8_t buflen = VW_MAX_MESSAGE_LEN;

    if (vw_get_message(buf, &buflen)){
        if((int)buf[1]==ID_RECEPTOR && (int)buf[2]==ID){
          char m[TAM-3]="H";
          int i;
          digitalWrite(13, true);
          digitalWrite(verde, HIGH);
          image_total += get_message(buf);
          digitalWrite(13, false);
          delay(500);
          digitalWrite(verde, LOW);
      }
    } 
    if(image_total.length() == 16*16){
      for(int j = 0; j<16*16; j++){
        if(image_total[j]=='0'){
          Serial.print("▢");
        }
        else{
          Serial.print("█");
        }
        if((j+1)%16==0){
          Serial.println("");
        }
      }
      Serial.println(image_total);
      image_total = "";
    }
}
