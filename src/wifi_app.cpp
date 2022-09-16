#include <esp_now.h>
#include <WiFi.h>
#include "define.h"
#include "obd2_app.hpp"

// WiFiと書いてあるが、ESP-NOWを使用する

esp_now_peer_info_t wifislave;
uint8_t wifircvbuf[CAN_PACKETSIZE * OBD2PKT_NUM];
bool wifisndflag = false;
bool wifircvflag = false;

// 送信コールバック
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  wifisndflag = false;
}

// 受信コールバック
void ESPnowOnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  if (wifircvflag == true) return; // 受信データが刈り取られていなければ廃棄
  for (uint8_t i = 0; i < (CAN_PACKETSIZE * OBD2PKT_NUM); i++) {
    wifircvbuf[i] = data[i];
  }
  wifircvflag = true;
}

void wifi_init() {
  // ESP-NOW初期化
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success");
  } else {
    Serial.println("ESPNow Init Failed");
    ESP.restart();
  }

  // マルチキャスト用Slaveを送信先に登録
  memset(&wifislave, 0, sizeof(wifislave));
  for (uint8_t i = 0; i < 6; i++) {
    wifislave.peer_addr[i] = (uint8_t)0xff;
  }  
  esp_err_t addStatus = esp_now_add_peer(&wifislave);
  if (addStatus == ESP_OK) {
    Serial.println("Pair success");
  }

  // ESP-NOWコールバック登録
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(ESPnowOnDataRecv);
}

void wifi_snd(uint8_t *buff) {
  if (wifisndflag == false) {
    esp_err_t result = esp_now_send(wifislave.peer_addr, buff, CAN_PACKETSIZE * OBD2PKT_NUM);
    wifisndflag = true;
  }
}

bool wifi_rcv(uint8_t *buff) {
  if (wifircvflag == true) {
    for (uint8_t i = 0; i < (CAN_PACKETSIZE * OBD2PKT_NUM); i++) {
      buff[i] = wifircvbuf[i];
    }
    wifircvflag = false;

    return true;
  }

  return false;
}
