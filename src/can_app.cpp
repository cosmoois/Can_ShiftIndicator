#include <Arduino.h>
#include "define.h"
#if !defined (CAN_OFF)
#include <CAN.h>

#if defined(LILYGO_TDisplay)
#define CAN_RX_PIN 37   // ダミー
#define CAN_TX_PIN 38   // ダミー
#elif defined(D1mini_ESP32)
#define CAN_RX_PIN 16
#define CAN_TX_PIN 17
#define DEBUG_LED 2
#endif

bool can_getpkt = false;
uint8_t can_rcvbuf[CAN_PACKETSIZE] = {0};
uint8_t can_sndbuf[CAN_PACKETSIZE] = {0};

void can_onReceive(int packetSize) {
  if (can_getpkt == true) return; // 受信データが刈り取られていなければ廃棄
#if defined(DEBUG_LED)
  digitalWrite(DEBUG_LED, HIGH);
#endif
  if ((!CAN.packetExtended()) && (!CAN.packetRtr())) {
    for (uint8_t i = 0; i < CAN_PACKETSIZE; i++) {
      if (i < packetSize) {
        can_rcvbuf[i] = CAN.read();
      } else {
        can_rcvbuf[i] = 0x00;
      }
    }
    can_getpkt = true;
  }
}

bool can_init() {
#if defined(DEBUG_LED)
  pinMode(DEBUG_LED, OUTPUT);
#endif
  CAN.setPins(CAN_RX_PIN, CAN_TX_PIN);
  if (!CAN.begin(500E3)) {
    Serial.println("Starting CAN failed!");
    return false;
  }
  CAN.onReceive(can_onReceive);

  CAN.filter(0x7e8);
  return true;
}

bool can_getPacket(uint8_t *buff) {
  if (can_getpkt == true) {
    for (uint8_t i = 0; i < CAN_PACKETSIZE; i++) {
      buff[i] = can_rcvbuf[i];
    }
#if defined(DEBUG_LED)
  digitalWrite(DEBUG_LED, LOW);
#endif
    can_getpkt = false;

    return true;
  }

  return false;
}

void can_sndPacket(uint8_t *buff) {
  for (uint8_t i = 0; i < CAN_PACKETSIZE; i++)
  {
    can_sndbuf[i] = buff[i];
  }
  CAN.beginPacket(0x7df, CAN_PACKETSIZE);
  CAN.write((const uint8_t *)can_sndbuf, CAN_PACKETSIZE);
  CAN.endPacket();
}
#endif
