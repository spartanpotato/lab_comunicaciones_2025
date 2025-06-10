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

// Clave publica: (e, n) = (3, 257)
// Clave privada: (d, n) = (171, 257)

#include <VirtualWire.h>

int verde = 4;
int const MESSAGE_TAM = 3; //Bytes
int const ID = 2;
int const ID_RECEPTOR = 2;
int const TAM = 7; //Bytes totales utilizados en el mensaje
int const TAM_ARRAY = 128;
uint8_t mensaje_completo[TAM_ARRAY];
bool recibido[43];

int d = 171;
int n = 257; // 17*43


// Seq  ID  ID_Receptor Message checksum
// []   []      []      [][][]     []

bool is_full() {
  for(int i = 0; i < 43; i++) { // Usar índice explícito
    if(!recibido[i]) {
      return false;
    }
  }
  return true;
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

uint8_t descifrar_RSA(uint8_t C, int d, int n) {
    uint32_t resultado = 1;  // Usar uint32_t
    uint32_t base = C;
    while (d > 0) {
        if (d % 2 == 1) {
            resultado = (resultado * base) % n;
        }
        base = (base * base) % n;
        d = d / 2;
    }
    return (uint8_t)resultado;
}


uint8_t crc8(uint8_t *d, uint8_t n) {
  uint8_t crc = 0x00;
  while(n--) { 
    crc ^= *d++; 
    for(uint8_t i=0; i<8; i++) crc = (crc<<1)^((crc&0x80)?0x07:0);
  }
  return crc;
}

String get_byte(uint8_t message){
  String bits = "";
  for(int i = 7; i>=0; i--){
    uint8_t bit =(message>>i)&1;
    bits+= String(bit);
  }
  return bits;
}

void get_message_asimetrico(uint8_t* message) {
  int sequence = (int)message[0];
  if(sequence >= 0 && sequence < TAM_ARRAY && recibido[sequence] == 0) { // Verificación más segura
    //String image = "";
    uint8_t decrypted_byte = 0b00000000;
    for(int i = 0; i < 3; i++) {
      uint8_t decrypted_byte = descifrar_RSA(message[3 + i], d, n);
      mensaje_completo[sequence * 3 + i] = decrypted_byte;
    }
    recibido[sequence] = 1;
  } else {
    Serial.print("Secuencia inválida: ");
    Serial.println(sequence);
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

void setup(){
    Serial.begin(9600);
    Serial.println("Configurando Recepcion");
    vw_set_ptt_inverted(true); 
    vw_setup(2000);
    vw_set_rx_pin(2);
    vw_rx_start();
    pinMode(verde, OUTPUT);
    for (int i = 0; i < TAM_ARRAY; i++) {
        mensaje_completo[i] = 0b00000000; // Asigna "H" a cada posición
    }
    for (int i = 0; i < 43; i++) {
        recibido[i] = 0;
    }
}

void loop() {
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  // Espera a recibir un mensaje
  if (vw_get_message(buf, &buflen)) {
    // Verifica si el mensaje es el esperado
    if ((int)buf[1] == ID_RECEPTOR && (int)buf[2] == ID) {
      // Verifica si el mensaje mantiene el checksum
      if (check_crc8(buf, buf[6])) {
        digitalWrite(13, true);
        digitalWrite(verde, HIGH);
        get_message_asimetrico(buf);
        Serial.println("Mensaje recibido: ");
        digitalWrite(13, false);
        delay(500);
        digitalWrite(verde, LOW);
      }
    }
  }

  //Imprime la imagen
  if (is_full()) {
    imprimirBits(mensaje_completo, TAM_ARRAY);
    for(int i = 0; i < 43; i++){
      recibido[i] = 0;
    }
  }
}
