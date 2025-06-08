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
int const TAM_ARRAY = (32*32)/24 + 1;
int const CLAVE = 3;
int cont;
String messages[TAM_ARRAY];
bool seq_recv[TAM_ARRAY];

void descifrarCesar(uint8_t mensaje[]) {
    for (int i = 3; i < TAM-1; i++) { 
        mensaje[i] = (mensaje[i] - CLAVE + 256) % 256; // Ajuste modular para evitar valores negativos
    }
}

void resetReceiver() {
    for (int i = 0; i < TAM_ARRAY; i++) {
        messages[i] = "H";
        seq_recv[i] = true;
    }
}

// Seq  ID  ID_Receptor Message checksum
// []   []      []      [][][]     []

// bool is_full(){
//   for(String elem: messages){
//     if(elem == "H"){
//       return false;
//     }
//   }
//   return true;
// }
bool is_full() {
  for(int i = 0; i < TAM_ARRAY; i++) { // Usar índice explícito
    if(messages[i] == "H") {
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
        String bits = "";
        for (int i = 3; i < 6; i++) {  // Bytes 3,4,5 (datos)
            for (int j = 7; j >= 0; j--) {
                bits += ((message[i] >> j) & 1) ? '1' : '0';
            }
        }
        messages[sequence] = bits;  // Guarda los 24 bits como String
        seq_recv[sequence] = false;
    }
}

String reconstructImage() {
    String fullImage = "";
    for (int i = 0; i < TAM_ARRAY; i++) {
        if (messages[i] != "H") {  // Si el paquete fue recibido
            fullImage += messages[i];  // Concatena los 24 bits
        }
    }
    // Asegura que tenga exactamente 1024 bits (32x32)
    fullImage = fullImage.substring(0, 32 * 32); 
    return fullImage;
}

void printImage() {
    String image = reconstructImage(); // Obtiene todos los bits en una String
    Serial.println("Imagen 32x32:");
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            int pos = i * 32 + j;
            if (pos < image.length()) {
                Serial.print(image[pos]);
            } else {
                Serial.print("?"); // Si falta algún bit
            }
        }
        Serial.println(); // Salto de línea cada 32 bits
    }
    Serial.println("¡Imagen completa!");
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
    cont = 0;
    for (int i = 0; i < TAM_ARRAY; i++) {
        seq_recv[i] = true;
        messages[i] = "H"; // Asigna "H" a cada posición
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
    printImage(); // ¡Ahora imprime correctamente en 32x32!
    resetReceiver(); // Reinicia para una nueva imagen
  }
  
  /*
  if (vw_get_message(buf, &buflen)) {
    // Verifica si el mensaje es el esperado
    if ((int)buf[1] == ID_RECEPTOR && (int)buf[2] == ID) {
      descifrarCesar(buf);
      // Verifica si el mensaje mantiene el checksum
      if (check_crc8(buf, buf[6]) && isnt_checked(buf)) {
        digitalWrite(13, true);
        digitalWrite(verde, HIGH);
        get_message(buf);
        digitalWrite(13, false);
        delay(500);
        digitalWrite(verde, LOW);
      }
    }
    */

  //Imprime la imagen
  // if (is_full()) {
  //   Serial.println("Imagen:");
  //   for (String elem : messages) {
  //     for (char c : elem) {
  //       if (c == '0') {
  //         Serial.print("0");
  //         cont++;
  //       } else {
  //         Serial.print("1");
  //         cont++;
  //       }
  //       if ((cont) == 32) {
  //         Serial.println("");
  //         cont = 0;
  //       }
  //     }
  //   }
  //   Serial.println("¡LISTO!");
  //   // memset(messages,"H",sizeof(messages));
  //   for (int i = 0; i < TAM_ARRAY; i++) {
  //       seq_recv[i] = true;
  //       messages[i] = "H";
  //   }
  // }
}

