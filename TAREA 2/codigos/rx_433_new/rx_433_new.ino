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
String messages[TAM_ARRAY];


// Seq  ID  ID_Receptor Message checksum
// []   []      []      [][][]     []

bool is_full(){
  for(String elem: messages){
    if(elem == "H"){
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

uint8_t crc8(const uint8_t *d, uint8_t n) {
  uint8_t crc = 0x00;
  while(n--) { 
    crc ^= *d++; 
    for(uint8_t i=0; i<TAM; i++) crc = (crc<<1)^((crc&0x80)?0x07:0);
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

// Saca los bits del mensaje
void get_message(uint8_t* message){
  int sequence = (int)message[0];
  //String checksum = get_byte(message[TAM-1]);
  String image = "";
  for(int i = 3; i<=TAM-1; i++){
    image+= get_byte(message[i]);
  }
  if(sequence < 0 || sequence >= TAM_ARRAY){
    Serial.print("Secuencia fuera de rango: ");
    Serial.println(sequence);
    return;
  }
  messages[sequence] = image;
  // return image;
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
        messages[i] = "H"; // Asigna "H" a cada posición
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
      if (check_crc8(buf, buf[7])) {
        digitalWrite(13, true);
        digitalWrite(verde, HIGH);
        get_message(buf);
        digitalWrite(13, false);
        delay(500);
        digitalWrite(verde, LOW);
      }
    }
  }

  //Imprime la imagen
  if (is_full()) {
    Serial.println("Imagen:");
    int cont = 0;
    for (String elem : messages) {
      for (char c : elem) {
        if (c == '0') {
          Serial.print("# ");
          cont++;
        } else {
          Serial.print("  ");
          cont++;
        }
        if ((cont) == 32) {
          Serial.println("");
          cont = 0;
        }
      }
    }
    Serial.println("");
    for (int i = 0; i < TAM_ARRAY; i++) {
      messages[i] = "H";
    }
  }
}