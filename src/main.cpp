#include <Arduino.h>
#include "define.h"
#include "can_app.hpp"
#include "obd2_app.hpp"
#include "wifi_app.hpp"
#include "display_app.hpp"

hw_timer_t * timer = NULL;

#if defined(LILYGO_TDisplay)
#define SW1_PIN 0   // onboard switch
#elif defined(D1mini_ESP32)
// SW none
#elif defined(M5STAMPC3_13) || defined(M5STAMPC3_20)
#define SW1_PIN 3   // onboard switch
#endif

int shift_pos = 1;  // シフトポジション

#define FUEL_BUF_CNT 5
float fuel_percent_buf[FUEL_BUF_CNT] = {-99};
float fuel_percent_fix = -99;
int fuel_pos = 0;
bool fuel_valid = false;

bool CAN2WiFibridge = false;    // CAN(Response)をWiFi経由で送信許可
bool demo_mode = false;

bool cansnd_ena = false;
bool wifisnd_ena = false;
uint32_t wificnt = 0;

uint8_t get_canbuf[CAN_PACKETSIZE] = {0};
uint8_t set_canbuf[CAN_PACKETSIZE] = {0};
uint8_t get_wifibuf[8][CAN_PACKETSIZE] = {0};
uint8_t set_wifibuf[8][CAN_PACKETSIZE] = {0};

// 車両パラメータ
const double tireround = (215 * 0.45 * 2 + 18 * 25.4) * 3.1416;  // タイヤ円周：215/45R18
const double finalratio = 3.804;  // 最終減速比
const double gearratio[] = { 3.487, 1.992, 1.449, 1.000, 0.707, 0.600 };  // 変速比(Rを除く)
int gearnum = sizeof(gearratio) / sizeof(gearratio[0]);

int find_nearest(double target, const double* a, size_t n);
void update_demo();

void IRAM_ATTR onTimer() {
#if defined(SW1_PIN)
  // バックライト輝度変更
  static bool sw_on = false;
  static int sw_cnt = 0;
  if (digitalRead(SW1_PIN) == LOW) {
    sw_on = true;
    sw_cnt++;
    if (sw_cnt >= 200) {  // 長押し：1000ms
      sw_cnt = 0;
      if (demo_mode == false) {
        demo_mode = true;
      } else {
        ESP.restart();
      }
    }
  } else {
    if (sw_on == true) {
      if ((sw_cnt > 0) && (sw_cnt <= 60)) { // 短押し：5ms-300ms
        display_brightness_shift();
      }
    }
    sw_on = false;
    sw_cnt = 0;
  }
#endif

  static uint32_t cmd_count = 0;
  if (CAN2WiFibridge == true) {
    wifisnd_ena = true;

    // 自動受信しないIDを定期的に問い合わせる
    // 下記の周期は、実車でのCAN負荷を考慮すること
    switch (cmd_count % (3 * 1000 / 5)) // 3秒周期
    {
    case 0:
      obd2_MakeStandardPacket(set_canbuf, 0x2F); // 残燃料(%)
      // obd2_MakeCustomPacket(set_canbuf, 0x61B1); // 残燃料(l)
      cansnd_ena = true;
      break;
    case 200:
      obd2_MakeCustomPacket(set_canbuf, 0x042D); // DPF PM Generated
      cansnd_ena = true;
      break;
    case 400:
      obd2_MakeCustomPacket(set_canbuf, 0x4028); // バッテリーSOC
      cansnd_ena = true;
      break;

    default:
      break;
    }
  }

  switch (cmd_count % (3 * 1000 / 5)) // 3秒周期
  {
  case 0:
    // 残燃料はセンサーの値をそのまま返しているため加減速で変化してしまう
    // 下記は簡易的に安定状態を取得する処理
    if (obd2_fuel_per >= 0) { // 残燃料受信済
      if (fuel_percent_fix < 0) { // 初回判定
        fuel_percent_fix = obd2_fuel_per;
      } else {
        fuel_percent_buf[fuel_pos] = obd2_fuel_per;
        fuel_pos = (fuel_pos + 1) % FUEL_BUF_CNT;
        fuel_valid = true;
        for (uint8_t i = 0; i < FUEL_BUF_CNT; i++) {
          if (obd2_fuel_per != fuel_percent_buf[i]) {
            fuel_valid = false;
          }
        }
        if (fuel_valid == true) {
          fuel_percent_fix = obd2_fuel_per;
        }
      }
    }
    break;

  default:
    break;
  }

  cmd_count++;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Shift Indicator");

#if !defined(WIFI_OFF)
  wifi_init();
#endif

  display_init();

#if !defined (CAN_OFF)
  can_init();
#endif

#if defined(SW1_PIN)
  pinMode(SW1_PIN, INPUT_PULLUP);
// #if defined(M5STAMPC3_13) || defined(M5STAMPC3_20)
//   pinMode(SW1_PIN, INPUT_PULLUP);
// #elif defined(LILYGO_TDisplay)
//   pinMode(SW1_PIN, INPUT);
// #endif
#endif

  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 5 * 1000, true); // 5ms周期
  timerAlarmEnable(timer);
}

void loop() {
  bool disp_refresh = false;
  uint16_t PIDcode = 0;
  uint8_t prmA = 0;
  uint8_t prmB = 0;

#if !defined(WIFI_OFF)
  // WiFi受信
  if (wifi_rcv(get_wifibuf[0]) == true) {
#ifdef VIEW_DEBUG
    wificnt++;
#endif
    // Responseパケット判定
    for (uint8_t i = 0; i < OBD2PKT_NUM; i++)
    {
      if (obd2_ChkResponse(get_wifibuf[i], &PIDcode, &prmA, &prmB) == true) {
        uint8_t dmyidx = 0;
        if (obd2_AnalysisPacket(PIDcode, prmA, prmB, &dmyidx) == true) {
          disp_refresh = true;
        }
      }
    }
  }
#endif

#if !defined (CAN_OFF)
  // CAN受信(Response)
  if (can_getPacket(get_canbuf) == true) {
    // Responseパケット判定
    if (obd2_ChkResponse(get_canbuf, &PIDcode, &prmA, &prmB) == true) {
      uint8_t pktidx = 0;
      if (obd2_AnalysisPacket(PIDcode, prmA, prmB, &pktidx) == true) {
        for (uint8_t i = 0; i < CAN_PACKETSIZE; i++)
        {
          set_wifibuf[pktidx][i] = get_canbuf[i];
        }
        CAN2WiFibridge = true;
        
        disp_refresh = true;
      }
    }
  }

#if !defined(WIFI_OFF)
  if (wifisnd_ena == true) {
    wifi_snd(set_wifibuf[0]);
    wifisnd_ena = false;
  }
#endif

  if (cansnd_ena == true) {
    can_sndPacket(set_canbuf);
    cansnd_ena = false;
  }
#endif

#ifdef FORCE_DEMO
  demo_mode = true;
#endif

  if (demo_mode == true) {
    delay(100);
    update_demo();
    disp_refresh = true;
#ifdef VIEW_DEBUG
    wificnt++;
#endif
  }

#if !defined(DISPLAY_OFF)
  if (disp_refresh == true) {
    if (obd2_kmh == 0) {
      shift_pos = 1;
    } else {
      double ratio = (tireround * obd2_rpm) / (obd2_kmh * finalratio) * 60 / 1000000;
      shift_pos = find_nearest(ratio, gearratio, gearnum);
    }

    display_inpane_draw(obd2_rpm, obd2_kmh, shift_pos);
  }
#endif
}

int find_nearest(double target, const double* a, size_t n) {
  double result = *a;
  double nearest_err = abs(*a - target);
  for (--n, ++a; n > 0; --n, ++a) {
    double err = abs(*a - target);
    if (err < nearest_err) { nearest_err = err; result = *a; }
  }

  for (uint8_t i = 0; i < 6; i++) {
    if (gearratio[i] == result) return i + 1;
  }

  return 0;
}

void update_demo() {
  static float cnt = 5;
  if (obd2_kmh > 102)  cnt = 5;
  obd2_kmh = cnt;
  if (obd2_kmh < 20) {
    obd2_rpm = (float)(5000 - 800) / 20 * (cnt - 5) + 800;
  } else if (obd2_kmh < 40) {
    obd2_rpm = (float)(3300 - 1100) / 20 * (cnt - 20) + 1100;
  } else if (obd2_kmh < 60) {
    obd2_rpm = (float)(3100 - 1600) / 20 * (cnt - 40) + 1600;
  } else if (obd2_kmh < 80) {
    obd2_rpm = (float)(3000 - 1600) / 20 * (cnt - 60) + 1600;
  } else if (obd2_kmh < 100) {
    obd2_rpm = (float)(2600 - 1700) / 20 * (cnt - 80) + 1700;
  }
  cnt += 1.7;

  fuel_percent_fix = 84.3;
  obd2_dpf_gen = 5.84;
  obd2_batt_soc = 86;
  obd2_eng_temp = 90;
}
