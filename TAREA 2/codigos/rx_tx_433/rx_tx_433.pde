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
++++++++++++++++++++++++++++++++++++++++++++++
Configuracion basica para modulo transmisor RT 11
Utiliza libreria VirtualWire.h
pin 01 entrada desde Arduino pin digital 3
pin 02 Tierra
pin 07 tierra
pin 08 antena externa
pin 09 tierra
pin 10 5v
*/






#include <VirtualWire.h>
const char *msg = "H";
void setup(){
    Serial.begin(9600);
    pinMode(13, OUTPUT);
    vw_set_ptt_inverted(true); 
    vw_setup(2000);
    vw_set_rx_pin(2);
    vw_set_tx_pin(3);
    vw_rx_start();
}

void loop(){
    uint8_t buf[VW_MAX_MESSAGE_LEN];
    uint8_t buflen = VW_MAX_MESSAGE_LEN;

    if (vw_get_message(buf, &buflen)){
        char m[5]="H";
	      int i;
        digitalWrite(13, true);
	      for (i = 0; i <= 5; i++){
	      m[i] = (char)buf[i];

	       }
        Serial.print("Mensaje Recibido = ");
        Serial.println(m);
        digitalWrite(13, false);
    }
    else{
      msg = "1";
      vw_send((uint8_t *)msg, strlen(msg));
      vw_wait_tx();
      delay(1000);
      Serial.println("mensaje enviado");
    }

}
