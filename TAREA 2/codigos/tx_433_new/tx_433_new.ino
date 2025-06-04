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

const char* mensajeCompleto = "11111111111111111111111111111111"
"11111111111111111111111111111111"
"11111111111111111111111111111111"
"11111111111111111111111111111111"
"11111111111111111111111111111111"
"11111111111111111111111111111111"
"11111111111111111111111111111111"
"11111111111111111111111111111111"
"11111111110011111111011111111111"
"11111111000000111100000011111111"
"11111110000000011000000001111111"
"11111110000000000000000001111111"
"11111100000000000000000000111111"
"11111100000000000000000000111111"
"11111100000000000000000000111111"
"11111110000000000000000001111111"
"11111110000000000000000001111111"
"11111110000000000000000001111111"
"11111111000000000000000011111111"
"11111111100000000000000111111111"
"11111111110000000000001111111111"
"11111111111000000000011111111111"
"11111111111100000000111111111111"
"11111111111110000001111111111111"
"11111111111111000011111111111111"
"11111111111111111111111111111111"
"11111111111111111111111111111111"
"11111111111111111111111111111111"
"11111111111111111111111111111111"
"11111111111111111111111111111111"
"11111111111111111111111111111111"
"11111111111111111111111111111111";
const char* mensajeCompleno = "Este es un mensaje muy largo que no cabe en un solo paquete de VirtualWire.";

const int TAMANO_MENSAJE = 3;
const int TAMANO_TOTAL = 1 + 1 + 1 + 1 + TAMANO_MENSAJE; // +1 para CRC8
uint8_t paquete[TAMANO_TOTAL];

const uint8_t id_emisor = 2;
const uint8_t id_receptor = 2;

int azul = 4;

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
  int longitud_bits = strlen(mensajeCompleto);
  int longitud = longitud_bits / 8;
  int enviados = 0;
  uint8_t secuencia = 0;

  while (enviados < longitud_bits) {
    int bits_restantes = longitud_bits - enviados;
    int largoParcial = min(TAMANO_MENSAJE, bits_restantes / 8);

    paquete[0] = secuencia;
    paquete[1] = id_emisor;
    paquete[2] = id_receptor;


    for (int i = 0; i < largoParcial; i++) {
      uint8_t b = 0;
      for (int j = 0; j < 8; j++) {
        int bitIndex = enviados + i * 8 + j;
        if (bitIndex < longitud_bits) {
          b |= (mensajeCompleto[bitIndex] - '0') << (7 - j);
        }
      }
      paquete[3 + i] = b;
    }

    // Rellenar con ceros si largoParcial es menor que TAMANO_MENSAJE
    if (largoParcial < TAMANO_MENSAJE) {
      for (int i = largoParcial; i < TAMANO_MENSAJE; i++) {
        paquete[3 + i] = 0;  // Rellenar con 0
      }
    }

    paquete[3 + largoParcial] = crc8(paquete, 3 + largoParcial);

    vw_send(paquete, 4 + largoParcial); // + 1 para crc (listo)
    vw_wait_tx();

    Serial.print("Enviado: ");
    Serial.print(" SEQ: "); Serial.print(secuencia);
    Serial.print(" EMISOR: "); Serial.print(id_emisor);
    Serial.print(" RECEPTOR: "); Serial.print(id_receptor);
    Serial.print(" MSG: ");
    for (int i = 0; i < largoParcial; i++) {
      uint8_t b = paquete[3 + i];
      for (int j = 7; j >= 0; j--) {
        Serial.print((b >> j) & 1);
      }
    }
    Serial.println();

    //Serial.print(" CRC: "); Serial.print(paquete[3 + largoParcial], HEX);
    Serial.println();

    enviados += largoParcial * 8;
    if (largoParcial == 0 || enviados >= longitud_bits) {
      break; // Terminar el bucle si se han enviado todos los bits.
    }
    secuencia++;
    delay(1000);
  }

  delay(5000);
}


