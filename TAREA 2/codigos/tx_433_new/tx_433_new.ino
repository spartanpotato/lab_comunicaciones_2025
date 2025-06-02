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

const char* mensajeCompleto = "Este es un mensaje muy largo que no cabe en un solo paquete de VirtualWire.";

const int TAMANO_MENSAJE = 3;
const int TAMANO_TOTAL = 1 + 1 + 1 + TAMANO_MENSAJE + 1; // +1 para CRC8
uint8_t paquete[TAMANO_TOTAL];

const uint8_t id_emisor = 2;
const uint8_t id_receptor = 2;

uint8_t crc8(const uint8_t *d, uint8_t n) {
  uint8_t crc = 0x00;
  while(n--) { crc ^= *d++; for(uint8_t i=0; i<8; i++) crc = (crc<<1)^((crc&0x80)?0x07:0); }
  return crc;
}

void setup() {
  vw_set_ptt_inverted(true);
  vw_setup(2000);
  vw_set_tx_pin(2);
  Serial.begin(9600);
  Serial.println("Listo para enviar en bloques");
}

void loop() {
  int longitud = strlen(mensajeCompleto);
  int enviados = 0;
  uint8_t secuencia = 0;

  while (enviados < longitud) {
    int largoParcial = min(TAMANO_MENSAJE, longitud - enviados);

    paquete[0] = secuencia;
    paquete[1] = id_emisor;
    paquete[2] = id_receptor;

    for (int i = 0; i < largoParcial; i++) {
      paquete[3 + i] = (uint8_t)mensajeCompleto[enviados + i];
    }

    paquete[3 + largoParcial] = crc8(paquete, 3 + largoParcial);

    vw_send(paquete, 3 + largoParcial + 1);
    vw_wait_tx();

    Serial.print("Enviado: ");
    Serial.print(" SEQ: "); Serial.print(secuencia);
    Serial.print(" EMISOR: "); Serial.print(id_emisor);
    Serial.print(" RECEPTOR: "); Serial.print(id_receptor);
    Serial.print(" MSG: ");
    for (int i = 0; i < largoParcial; i++) {
      Serial.print((char)paquete[3 + i]);
    }
    Serial.print(" CRC: "); Serial.print(paquete[3 + largoParcial], HEX);
    Serial.println();

    enviados += largoParcial;
    secuencia++;
    delay(1000);
  }

  delay(5000);
}


