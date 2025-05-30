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
int const TAM = 6;

String get_message(char* message){
  int emisor = (int)message[1];
  if(emisor != ID_RECEPTOR) return "NaN";
  int receptor = (int)message[2];
  if(receptor != ID) return "NaN";
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
          for (i = 0; i <= TAM-3; i++){
              m[i] = (char)buf[i+3];
          }
          Serial.print("Mensaje Recibido = ");
          Serial.println(m);
          digitalWrite(13, false);
          delay(500);
          digitalWrite(verde, LOW);
      }
    } 
}
