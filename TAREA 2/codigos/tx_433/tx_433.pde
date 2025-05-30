/*
ver ::cl 20120520
Configuracion basica para modulo transmisor RT 11
Utiliza libreria VirtualWire.h
pin 01 entrada desde Arduino pin digital 2
pin 02 Tierra
pin 07 tierra
pin 08 antena externa
pin 09 tierra
pin 10 5v
*/


#include <VirtualWire.h>
const char *msg = "H";
void setup(){
    vw_set_ptt_inverted(true);
    vw_setup(2000);
    vw_set_tx_pin(2);    
   Serial.begin(9600);
   Serial.println("configurando envio");
}
void loop(){
   msg = "IoT";
   vw_send((uint8_t *)msg, strlen(msg));
   vw_wait_tx();
   delay(1000);
   Serial.println("mensaje enviado");
   
}
