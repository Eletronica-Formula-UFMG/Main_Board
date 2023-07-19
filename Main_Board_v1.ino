/*
  Codigo v1 da placa Main Board do sistema de aquisicao do TR08 da equipe Formula SAE UFMG.
  Realiza a leitura do barramento CAN e a descompactacao do dado de acordo com o emissor da
  mensagem atraves do ID da placa.
  Autor: Lucas Lourenco Reis Resende
  Data: 02/07/2023  
*/

#include <ESP32CAN.h>
#include <CAN_config.h>
#include <mySD.h>
#include <Wire.h>
#include <Arduino.h>

ext::File root1;

CAN_device_t CAN_cfg;

/*
  Nome: desCompactAcel
  Descricao: Funcao de tratamento do dado de aceleracao enviado pela placa de aceleracao apos a leitura
  de um sensor acelerometro MPU6050.
*/

float desCompactAcel(int8_t *buffer) {
  // Combina os 2 bytes em um valor inteiro de 16 bits
  int combinedBytes = ((int)buffer[0] << 8) | buffer[1];

  // Escala o valor de volta para o intervalo de 0,00 a 12,00
  // Esse intervalo foi escolhido devido ao range de +/- 6 G de aceleracao lateral
  float value = (float)combinedBytes * 12.0 / 256.0;

  // Para valores negativos eh necessario converter
  if (value < 0) {
    value = fabs(value) - 6.00;
  }

  return value;  // Retorna o valor original
}

/*
  Nome: desCompactTemp
  Descricao: Funcao de tratamento do dado de temperatura enviado pela placa de temperatura apos a leitura
  de um sensor infravermelho MLX90614.
*/

float desCompactTemp(byte *buffer) {
  // Combina os 2 bytes em um valor inteiro de 16 bits
  unsigned int combinedBytes = ((unsigned int)buffer[0] << 8) | buffer[1];

  // Escala o valor de volta para o intervalo de 0,00 a 300,00
  // Esse intervalo foi escolhido devido ao range de 300°C de temperatura no disco de freio
  float value = (float)combinedBytes * 300.0 / 65536;

  return value;  // Retorna o valor original
}

/*
  Nome: bytesToFloatExt
  Descricao: Funcao de tratamento do dado de extensometria enviado por uma das placas de extesometria apos a leitura
  de um extensometro posicionado no carro.
*/

float bytesToFloatExt(byte *byteArray) {
  // Cria um objeto union
  union {
    byte bytes[4];
    float floatValue;
  } converter;

  // Copia os bytes para o union
  for (int i = 0; i < 4; i++) {
    converter.bytes[i] = byteArray[i];
  }

  return converter.floatValue;  // Retorna o valor original
}

void trataFrame(CAN_frame_t *frame) {
  // Variaveis e vetores auxiliares
  int8_t compactedAcel[2];
  float dado = 0.0;
  float dadoX = 0.0;
  float dadoY = 0.0;
  float dadoZ = 0.0;
  float dadoExt = 0.0;
  byte bytesExt[sizeof(float)];
  byte compactedTemp[2];

  // Testa o ID da mensagem e chama a funcao de tratamento de acordo com o tipo de dado
  switch (frame->MsgID) {

    case 0x301:  // Dado de acelerometro
      for (int i = 0; i < 8; i++) {
        if (i >= 1 && i % 2 != 0 && i < 7) {  // So descompacta se for um indice impar >=1
          compactedAcel[0] = frame->data.u8[i];
          compactedAcel[1] = frame->data.u8[i + 1];
          dado = desCompactAcel(compactedAcel);
          if (i == 1) {
            dadoX = dado;
          }
          if (i == 3) {
            dadoY = dado;
          }
          if (i == 5) {
            dadoZ = dado;
          }
        }
      }
      switch (frame->data.u8[0]) {
        case 0x1:
          Serial.println("Dado Aceleracao 1");
          Serial.print(dadoX);
          Serial.print("\t");
          Serial.print(dadoY);
          Serial.print("\t");
          Serial.println(dadoZ);
          root1 = SD.open("Acel1.txt", FILE_WRITE);
          root1.print(dadoX);
          root1.print("\t");
          root1.print(dadoY);
          root1.print("\t");
          root1.print(dadoZ);
          root1.print("\t");
          root1.close();
          break;
        case 0x2:
          Serial.println("Dado Aceleracao 2");
          Serial.print(dadoX);
          Serial.print("\t");
          Serial.print(dadoY);
          Serial.print("\t");
          Serial.println(dadoZ);
          root1 = SD.open("Acel2.txt", FILE_WRITE);
          root1.print(dadoX);
          root1.print("\t");
          root1.print(dadoY);
          root1.print("\t");
          root1.print(dadoZ);
          root1.print("\t");
          root1.close();
          break;
        case 0x3:
          Serial.println("Dado Vel. Ang. 1");
          Serial.print(dadoX);
          Serial.print("\t");
          Serial.print(dadoY);
          Serial.print("\t");
          Serial.println(dadoZ);
          root1 = SD.open("Acel1.txt", FILE_WRITE);
          root1.print(dadoX);
          root1.print("\t");
          root1.print(dadoY);
          root1.print("\t");
          root1.print(dadoZ);
          root1.print("\t");
          root1.println(millis());
          root1.close();
          break;
        case 0x4:
          Serial.println("Dado Vel. Ang. 2");
          Serial.print(dadoX);
          Serial.print("\t");
          Serial.print(dadoY);
          Serial.print("\t");
          Serial.println(dadoZ);
          root1 = SD.open("Acel2.txt", FILE_WRITE);
          root1.print(dadoX);
          root1.print("\t");
          root1.print(dadoY);
          root1.print("\t");
          root1.print(dadoZ);
          root1.print("\t");
          root1.println(millis());
          root1.close();
          break;
        default:
          Serial.println();
      }
      break;

    case 0x302:  // Dado de extensometria dianteira 1
      bytesExt[0] = frame->data.u8[1];
      bytesExt[1] = frame->data.u8[2];
      bytesExt[2] = frame->data.u8[3];
      bytesExt[3] = frame->data.u8[4];
      dadoExt = bytesToFloatExt(bytesExt);  // Chama a funcao para converter
      switch (frame->data.u8[0]) {
        case 0x1:
          Serial.print(dadoExt);
          Serial.print("\t");
          root1 = SD.open("ExtD1.txt", FILE_WRITE);
          root1.print(dadoExt);
          root1.print("\t");
          root1.close();
          break;
        case 0x2:
          Serial.print(dadoExt);
          Serial.print("\t");
          root1 = SD.open("ExtD1.txt", FILE_WRITE);
          root1.print(dadoExt);
          root1.print("\t");
          root1.close();
          break;
        case 0x3:
          Serial.print(dadoExt);
          Serial.print("\t");
          root1 = SD.open("ExtD1.txt", FILE_WRITE);
          root1.print(dadoExt);
          root1.print("\t");
          root1.close();
          break;
        case 0x4:
          Serial.println(dadoExt);
          root1 = SD.open("ExtD1.txt", FILE_WRITE);
          root1.print(dadoExt);
          root1.println(millis());
          root1.close();
          break;
        default:
          Serial.println();
      }
      break;

    case 0x303:  // Dado de extensometria dianteira 2
      bytesExt[0] = frame->data.u8[1];
      bytesExt[1] = frame->data.u8[2];
      bytesExt[2] = frame->data.u8[3];
      bytesExt[3] = frame->data.u8[4];
      dadoExt = bytesToFloatExt(bytesExt);  // Chama a funcao para converter
      switch (frame->data.u8[0]) {
        case 0x1:
          Serial.print(dadoExt);
          Serial.print("\t");
          root1 = SD.open("ExtD2.txt", FILE_WRITE);
          root1.print(dadoExt);
          root1.print("\t");
          root1.close();
          break;
        case 0x2:
          Serial.print(dadoExt);
          Serial.print("\t");
          root1 = SD.open("ExtD2.txt", FILE_WRITE);
          root1.print(dadoExt);
          root1.print("\t");
          root1.close();
          break;
        case 0x3:
          Serial.print(dadoExt);
          Serial.print("\t");
          root1 = SD.open("ExtD2.txt", FILE_WRITE);
          root1.print(dadoExt);
          root1.print("\t");
          root1.close();
          break;
        case 0x4:
          Serial.println(dadoExt);
          root1 = SD.open("ExtD2.txt", FILE_WRITE);
          root1.print(dadoExt);
          root1.println(millis());
          root1.close();
          break;
        default:
          Serial.println();
      }
      break;

    case 0x304:  // Dado de extensometria traseira
      bytesExt[0] = frame->data.u8[1];
      bytesExt[1] = frame->data.u8[2];
      bytesExt[2] = frame->data.u8[3];
      bytesExt[3] = frame->data.u8[4];
      dadoExt = bytesToFloatExt(bytesExt);  // Chama a funcao para converter
      switch (frame->data.u8[0]) {
        case 0x1:
          Serial.print(dadoExt);
          Serial.print("\t");
          root1 = SD.open("ExtT.txt", FILE_WRITE);
          root1.print(dadoExt);
          root1.print("\t");
          root1.close();
          break;
        case 0x2:
          Serial.print(dadoExt);
          Serial.print("\t");
          root1 = SD.open("ExtT.txt", FILE_WRITE);
          root1.print(dadoExt);
          root1.print("\t");
          root1.close();
          break;
        case 0x3:
          Serial.print(dadoExt);
          Serial.print("\t");
          root1 = SD.open("ExtT.txt", FILE_WRITE);
          root1.print(dadoExt);
          root1.print("\t");
          root1.close();
          break;
        case 0x4:
          Serial.println(dadoExt);
          root1 = SD.open("ExtT.txt", FILE_WRITE);
          root1.print(dadoExt);
          root1.println(millis());
          root1.close();
          break;
        default:
          Serial.println();
      }
      break;

    case 0x305:  // Dado de temperatura
      for (int i = 0; i < 8; i++) {
        if (i >= 1 && i % 2 != 0 && i < 7) {  // So descompacta se for um indice impar >=1
          compactedTemp[0] = frame->data.u8[i];
          compactedTemp[1] = frame->data.u8[i + 1];
          dado = desCompactTemp(compactedTemp);
          if (i == 1) {
            dadoX = dado;
          }
          if (i == 3) {
            dadoY = dado;
          }
          if (i == 5) {
            dadoZ = dado;
          }
        }
      }
      switch (frame->data.u8[0]) {
        case 0x1:
          Serial.println("Dado Temp 1, 2, 3");
          Serial.print(dadoX);
          Serial.print("\t");
          Serial.print(dadoY);
          Serial.print("\t");
          Serial.println(dadoZ);
          root1 = SD.open("Temp.txt", FILE_WRITE);
          root1.print(dadoX);
          root1.print("\t");
          root1.print(dadoY);
          root1.print("\t");
          root1.print(dadoZ);
          root1.print("\t");
          root1.close();
          break;
        case 0x2:
          Serial.println("Dado Temp 4, 5, 6");
          Serial.print(dadoX);
          Serial.print("\t");
          Serial.print(dadoY);
          Serial.print("\t");
          Serial.println(dadoZ);
          root1 = SD.open("Temp.txt", FILE_WRITE);
          root1.print(dadoX);
          root1.print("\t");
          root1.print(dadoY);
          root1.print("\t");
          root1.print(dadoZ);
          root1.print("\t");
          root1.println(millis());
          root1.close();
          break;
        default:
          Serial.println();
      }
      break;
    default:
      Serial.println();
  }
}

/*
  Nome: setup
  Descricao: Incializa o modulo CAN e SD e seta os parametros necessarios para a transmissao
*/

void setup() {
  Serial.begin(115200);

  /* initialize SD library with SPI pins */
  if (!SD.begin(15, 23, 19, 18)) {
    SD.begin(15, 23, 19, 18);
    Serial.println("initialization failed!");
    //return;
  }
  Serial.println("initialization done.");
  root1 = SD.open("/");
  if (root1) {
    root1.close();
  } else {
    Serial.println("error opening .txt");
  }

  /* Configura a taxa de transmissão e os pinos Tx e Rx */
  CAN_cfg.speed = CAN_SPEED_1000KBPS;
  CAN_cfg.tx_pin_id = GPIO_NUM_4;  //GPIO 5 (Pino 6) = CAN TX
  CAN_cfg.rx_pin_id = GPIO_NUM_5;  //GPI 35 (Pino 30) = CAN RX

  /* Cria uma fila de até 10 elementos para receber os frames CAN */
  CAN_cfg.rx_queue = xQueueCreate(10, sizeof(CAN_frame_t));

  /* Inicializa o módulo CAN */
  ESP32Can.CANInit();
}

/*
  Nome: loop
  Descricao: Faz a leitura do barramento e a chamada de funcoes de tratamento dos dados de acordo com a placa
*/

void loop() {
  root1.close();
  // Criacao do arquivo
  root1 = SD.open("ExtDiant1.txt", FILE_WRITE);

  /* Declaração dos pacotes CAN */
  CAN_frame_t frame_1;
  CAN_frame_t frame_2;
  CAN_frame_t frame_3;
  CAN_frame_t frame_4;

  /* Recebe o primeiro pacote CAN na fila */
  if (xQueueReceive(CAN_cfg.rx_queue, &frame_1, 3 * portTICK_PERIOD_MS) == pdTRUE) {
    trataFrame(&frame_1);
  } else if (xQueueReceive(CAN_cfg.rx_queue, &frame_2, 3 * portTICK_PERIOD_MS) == pdTRUE) {
    trataFrame(&frame_2);
  } else if (xQueueReceive(CAN_cfg.rx_queue, &frame_3, 3 * portTICK_PERIOD_MS) == pdTRUE) {
    trataFrame(&frame_3);
  } else if (xQueueReceive(CAN_cfg.rx_queue, &frame_4, 3 * portTICK_PERIOD_MS) == pdTRUE) {
    trataFrame(&frame_4);
  }

  delay(100);
}
