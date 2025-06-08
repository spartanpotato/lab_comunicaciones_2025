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
int const TAM = 7; //Bytes totales utilizados en el mensaje
int const TAM_ARRAY = 128;
int const CLAVE = 3;
uint8_t messages[TAM_ARRAY];
bool seq_recv[43];

void descifrarCesar(uint8_t mensaje[]) {
    for (int i = 3; i < TAM-1; i++) { 
        mensaje[i] = (mensaje[i] - CLAVE + 256) % 256; // Ajuste modular para evitar valores negativos
    }
}

void resetReceiver() {
    /*for (int i = 0; i < TAM_ARRAY; i++) {
        messages[i] = 0b00000000;
    }*/
    for (int i = 0; i < 43; i++) {
        seq_recv[i] = true;
    }
}

// Seq  ID  ID_Receptor Message checksum
// []   []      []      [][][]     []


bool is_full() {
  for(int i = 0; i < 43; i++) { // Usar índice explícito
    if(seq_recv[i]) {
      return false;
    }
  }
  return true;
}

bool isnt_checked(uint8_t *d){
  int sequence = (int)d[0];
  return seq_recv[sequence];
}

bool check_crc8(uint8_t *d, uint8_t crc_recv){
  uint8_t crc_calc = crc8(d, TAM-1); // Calcula CRC de los primeros 6 bytes
  if(crc_recv == crc_calc){
    return true;
  }
  else{
    Serial.print("CRC no valido: ");
    Serial.print(crc_recv);
    Serial.print(" != ");
    Serial.println(crc_calc);
    return false;
  }
}

uint8_t crc8(uint8_t *d, uint8_t n) {
  uint8_t crc = 0x00;
  while(n--) { 
    crc ^= *d++; 
    for(uint8_t i=0; i<8; i++) crc = (crc<<1)^((crc&0x80)?0x07:0);
  }
  return crc;
}

void get_message(uint8_t* message) {
    int sequence = (int)message[0];
    if (sequence >= 0 && sequence < TAM_ARRAY) {
        for (int i = 0; i < 3; i++) {  // Bytes 3,4,5 (datos)
            messages[sequence * 3 + i] = message[3 + i];
        }
        seq_recv[sequence] = false;
    }
}

void imprimirBits(const uint8_t *array, const size_t TAMANO_ARRAY) {
  for(size_t i = 0; i < TAMANO_ARRAY; i++) {
    // Recorremos cada bit del byte, empezando por el más significativo (bit 7)
    for(int8_t bit = 7; bit >= 0; bit--) {
      if(array[i] & (1 << bit)) {
        Serial.print("1");
      } else {
        Serial.print("0");
      }
      
      // Salto de línea cada 32 bits (contamos bits impresos)
      static uint16_t contadorBits = 0;
      contadorBits++;
      if(contadorBits % 32 == 0) {
        Serial.println();
      }
    }
  }
  // Salto de línea final si no terminó en múltiplo de 32
  if(TAMANO_ARRAY * 8 % 32 != 0) {
    Serial.println();
  }
}

// void get_message(uint8_t* message) {
//   int sequence = (int)message[0];
//   if(sequence >= 0 && sequence < TAM_ARRAY) { // Verificación más segura
//     String image = "";
//     for(int i = 3; i < TAM-1; i++) {
//       image += get_byte(message[i]);
//     }
//     seq_recv[sequence] = false;
//     messages[sequence] = image;
//   } else {
//     Serial.print("Secuencia inválida: ");
//     Serial.println(sequence);
//   }
// }
void setup(){
    Serial.begin(9600);
    Serial.println("Configurando Recepcion");
    vw_set_ptt_inverted(true); 
    vw_setup(2000);
    vw_set_rx_pin(2);
    vw_rx_start();
    pinMode(verde, OUTPUT);
    for (int i = 0; i < TAM_ARRAY; i++) {
        messages[i] = 0b00000000; // Asigna "H" a cada posición
    }
    for (int i = 0; i < 43; i++) {
        seq_recv[i] = true;
    }
}

void loop() {
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  // Espera a recibir un mensaje
  if (vw_get_message(buf, &buflen)) {
      if ((int)buf[1] == ID_RECEPTOR && (int)buf[2] == ID) {
          descifrarCesar(buf);
          if (check_crc8(buf, buf[6]) && isnt_checked(buf)) {
              get_message(buf); // Almacena en messages[]
              digitalWrite(verde, HIGH);
              delay(500);
              digitalWrite(verde, LOW);
            }
        }
  }
  if (is_full()) {
    imprimirBits(messages, TAM_ARRAY); // ¡Ahora imprime correctamente en 32x32!
    resetReceiver(); // Reinicia para una nueva imagen
  }
}

