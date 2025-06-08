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

const uint8_t mensajeCompleto[128] = {
  0b11111111, 0b11111111, 0b11111111, 0b11111111,
  0b11111111, 0b11111111, 0b11111111, 0b11111111,
  0b11111111, 0b11111111, 0b11111111, 0b11111111,
  0b11111111, 0b11111111, 0b11111111, 0b11111111,
  0b11111111, 0b11111111, 0b11111111, 0b11111111,
  0b11111111, 0b11111111, 0b11111111, 0b11111111,
  0b11111111, 0b11111111, 0b11111111, 0b11111111,
  0b11111111, 0b11111111, 0b11111111, 0b11111111,
  0b11111111, 0b11001111, 0b11110111, 0b11111111,
  0b11111111, 0b00000011, 0b11000000, 0b11111111,
  0b11111110, 0b00000001, 0b10000000, 0b01111111,
  0b11111110, 0b00000000, 0b00000000, 0b01111111,
  0b11111100, 0b00000000, 0b00000000, 0b00111111,
  0b11111100, 0b00000000, 0b00000000, 0b00111111,
  0b11111100, 0b00000000, 0b00000000, 0b00111111,
  0b11111110, 0b00000000, 0b00000000, 0b01111111,
  0b11111110, 0b00000000, 0b00000000, 0b01111111,
  0b11111110, 0b00000000, 0b00000000, 0b01111111,
  0b11111111, 0b00000000, 0b00000000, 0b11111111,
  0b11111111, 0b10000000, 0b00000001, 0b11111111,
  0b11111111, 0b11000000, 0b00000011, 0b11111111,
  0b11111111, 0b11100000, 0b00000111, 0b11111111,
  0b11111111, 0b11110000, 0b00001111, 0b11111111,
  0b11111111, 0b11111000, 0b00011111, 0b11111111,
  0b11111111, 0b11111100, 0b00111111, 0b11111111,
  0b11111111, 0b11111111, 0b11111111, 0b11111111,
  0b11111111, 0b11111111, 0b11111111, 0b11111111,
  0b11111111, 0b11111111, 0b11111111, 0b11111111,
  0b11111111, 0b11111111, 0b11111111, 0b11111111,
  0b11111111, 0b11111111, 0b11111111, 0b11111111,
  0b11111111, 0b11111111, 0b11111111, 0b11111111,
  0b11111111, 0b11111111, 0b11111111, 0b11111111
};


const int TAMANO_MENSAJE = 3;
const int TAMANO_TOTAL = 1 + 1 + 1 + 1 + TAMANO_MENSAJE; // +1 para CRC8
uint8_t paquete[TAMANO_TOTAL];

const int CLAVE = 3;

const uint8_t id_emisor = 2;
const uint8_t id_receptor = 2;

int azul = 4;

uint8_t crc8(const uint8_t *d, uint8_t n) {
  uint8_t crc = 0x00;
  while(n--) { crc ^= *d++; for(uint8_t i=0; i<8; i++) crc = (crc<<1)^((crc&0x80)?0x07:0); }
  return crc;
}

void cifrarCesar(uint8_t mensaje[]) {
    for (int i = 3; i < TAMANO_MENSAJE + 3; i++) { 
        mensaje[i] = (mensaje[i] + CLAVE) % 256; // Aplicamos desplazamiento con módulo
    }
}

void setup() {
  vw_set_ptt_inverted(true);
  vw_setup(2000);
  vw_set_tx_pin(2);
  Serial.begin(9600);
  Serial.println("Listo para enviar en bloques");
}

void loop() {
  int longitud_bytes = sizeof(mensajeCompleto);
  int longitud_bits = longitud_bytes * 8;
  int enviados = 0;
  uint8_t secuencia = 0;

  while (enviados < longitud_bits) {
    int bits_restantes = longitud_bits - enviados;
    int largoParcial = min(TAMANO_MENSAJE, bits_restantes / 8);

    memset(paquete, 0, TAMANO_TOTAL);

    paquete[0] = secuencia;
    paquete[1] = id_emisor;
    paquete[2] = id_receptor;


    for (int i = 0; i < largoParcial; i++) {    
      uint8_t M = mensajeCompleto[(enviados / 8) + i];
      paquete[3 + i] = M;
    }

    // Rellenar con ceros si largoParcial es menor que TAMANO_MENSAJE
    if (largoParcial < TAMANO_MENSAJE) {
      for (int i = largoParcial; i < TAMANO_MENSAJE; i++) {
        paquete[3 + i] = 0b00000000;  // Rellenar con 0
      }
    }

    paquete[3 + TAMANO_MENSAJE] = crc8(paquete, 3 + TAMANO_MENSAJE);

    cifrarCesar(paquete);

    vw_send(paquete, 4 + TAMANO_MENSAJE); // + 1 para crc (listo)
    vw_wait_tx();

    Serial.print("Enviado: ");
    Serial.print(" SEQ: "); Serial.print(secuencia);
    Serial.print(" EMISOR: "); Serial.print(id_emisor);
    Serial.print(" RECEPTOR: "); Serial.print(id_receptor);
    Serial.print(" MSG: ");
    for (int i = 0; i < TAMANO_MENSAJE; i++) {
      Serial.print(paquete[3 + i]);
    }

    Serial.print(" CRC: "); Serial.print(paquete[3 + TAMANO_MENSAJE]);
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


