#include <ESP32CAN.h>
#include <CAN_config.h>

CAN_device_t CAN_cfg;

float desCompactAcel(int8_t *buffer) {
  // Combina os 2 bytes em um valor inteiro de 16 bits
  int combinedBytes = ((int)buffer[0] << 8) | buffer[1];

  // Escala o valor de volta para o intervalo de 0,00 a 12,00
  float value = (float)combinedBytes * 12.0 / 256.0;
  //float value = (float)combinedBytes * 12.0 / 65536;
  if (value < 0) {  //Se o valor for negativo trata de um jeito
    value = fabs(value) - 6.00;
  }

  return value;
}

float desCompactTemp(byte *buffer) {
  // Combina os 2 bytes em um valor inteiro de 16 bits
  unsigned int combinedBytes = ((unsigned int)buffer[0] << 8) | buffer[1];

  // Escala o valor de volta para o intervalo de 0,00 a 12,00
  float value = (float)combinedBytes * 300.0 / 65536;
  return value;
}

float bytesToFloatExt(byte *byteArray) {
  union {
    byte bytes[4];
    float floatValue;
  } converter;

  // Copia os bytes para o union
  for (int i = 0; i < 4; i++) {
    converter.bytes[i] = byteArray[i];
  }
  return converter.floatValue;
}

void setup() {
  Serial.begin(115200);

  /* Configura a taxa de transmissão e os pinos Tx e Rx */
  CAN_cfg.speed = CAN_SPEED_1000KBPS;
  CAN_cfg.tx_pin_id = GPIO_NUM_4;  //GPIO 5 (Pino 6) = CAN TX
  CAN_cfg.rx_pin_id = GPIO_NUM_5;  //GPI 35 (Pino 30) = CAN RX

  /* Cria uma fila de até 10 elementos para receber os frames CAN */
  CAN_cfg.rx_queue = xQueueCreate(10, sizeof(CAN_frame_t));

  /* Inicializa o módulo CAN */
  ESP32Can.CANInit();
}

void loop() {
  /* Declaração dos pacotes CAN */
  CAN_frame_t frame_1;
  int8_t compactedAcel[2];
  float dado = 0.0;
  float dadoX, dadoY, dadoZ;

  //float floatValue;
  byte bytesExt[sizeof(float)];

  byte compactedTemp[2];

  /* Recebe o primeiro pacote CAN na fila */
  if (xQueueReceive(CAN_cfg.rx_queue, &frame_1, 3 * portTICK_PERIOD_MS) == pdTRUE) {

    switch (frame_1.MsgID) {
      case 0x301:  //Dado de acelerometro
        for (int i = 0; i < 8; i++) {
          if (i >= 1 && i % 2 != 0 && i < 7) {  //Só descompacta se for um índice ímpar >=1
            compactedAcel[0] = frame_1.data.u8[i];
            compactedAcel[1] = frame_1.data.u8[i + 1];
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
        break;

      case 0x302:  //Dado de extensometria
        bytesExt[0] = frame_1.data.u8[1];
        bytesExt[1] = frame_1.data.u8[2];
        bytesExt[2] = frame_1.data.u8[3];
        bytesExt[3] = frame_1.data.u8[4];
        dadoX = bytesToFloatExt(bytesExt);  // Chama a função para converter
        break;

      case 0x303:  //Dado de extensometria
        bytesExt[0] = frame_1.data.u8[1];
        bytesExt[1] = frame_1.data.u8[2];
        bytesExt[2] = frame_1.data.u8[3];
        bytesExt[3] = frame_1.data.u8[4];
        dadoX = bytesToFloatExt(bytesExt);  // Chama a função para converter
        break;

      case 0x304:  //Dado de extensometria
        bytesExt[0] = frame_1.data.u8[1];
        bytesExt[1] = frame_1.data.u8[2];
        bytesExt[2] = frame_1.data.u8[3];
        bytesExt[3] = frame_1.data.u8[4];
        dadoX = bytesToFloatExt(bytesExt);  // Chama a função para converter
        break;

      case 0x305:  //Dado de temperatura
        for (int i = 0; i < 8; i++) {
          if (i >= 1 && i % 2 != 0 && i < 7) {  //Só descompacta se for um índice ímpar >=1
            compactedTemp[0] = frame_1.data.u8[i];
            compactedTemp[1] = frame_1.data.u8[i + 1];
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
        break;
      default:
        Serial.println();
    }
    Serial.print(dadoX);
    Serial.print("\t");
    Serial.print(dadoY);
    Serial.print("\t");
    Serial.println(dadoZ);
  }
}
