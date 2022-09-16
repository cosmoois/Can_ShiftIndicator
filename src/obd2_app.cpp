#include <Arduino.h>

#include "define.h"
#include "obd2_app.hpp"

int obd2_eng_temp = -99;
int obd2_rpm = 0;
int obd2_kmh = 0;
float obd2_fuel_per = -99;
// int obd2_fuel_liter = -99;
int obd2_batt_soc = -99;
float obd2_dpf_gen = -99;

void obd2_MakeStandardPacket(uint8_t *buff, uint16_t PIDcode) {
  buff[0] = 0x02;
  buff[1] = 0x01;
  buff[2] = (PIDcode & 0xFF);
  for (uint8_t i = 3; i < CAN_PACKETSIZE; i++) {
    buff[i] = 0x00;
  }
}

void obd2_MakeCustomPacket(uint8_t *buff, uint16_t PIDcode) {
  buff[0] = 0x03;
  buff[1] = 0x22;
  buff[2] = (PIDcode >> 8);
  buff[3] = (PIDcode & 0xFF);
  for (uint8_t i = 4; i < CAN_PACKETSIZE; i++) {
    buff[i] = 0x00;
  }
}

bool obd2_ChkResponse(uint8_t *buff, uint16_t *PIDcode, uint8_t *prmA, uint8_t *prmB) {
  // Response
  if (buff[1] == 0x41) {  // show current data
    *PIDcode = (uint16_t)buff[2];
    *prmA = buff[3];
    *prmB = buff[4];
  } else if (buff[1] == 0x62) {  // response to service 22h request
    *PIDcode = ((uint16_t)buff[2] << 8) | (uint16_t)buff[3];
    *prmA = buff[4];
    *prmB = buff[5];
  }
  // etc
  else {
    return false;
  }

  return true;
}

bool obd2_AnalysisPacket(uint16_t PIDcode, uint8_t prmA, uint8_t prmB, uint8_t *pktidx) {
  switch(PIDcode) {
    case 0x0C:  // エンジン回転数
      obd2_rpm = (prmA * 256 + prmB) / 4;
      *pktidx = OBD2PKT_RPM;
      break;
    case 0x0D:  // 車速
      obd2_kmh = prmA;
      *pktidx = OBD2PKT_KMH;
      break;
    case 0x05:  // 水温
      obd2_eng_temp = prmA - 40;
      *pktidx = OBD2PKT_ENG_TEMP;
      break;
    case 0x2F:  // 残燃料(%)
      obd2_fuel_per = (float)(prmA) * 100 / 255;
      *pktidx = OBD2PKT_FUEL_PER;
      break;
    case 0x4028:  // バッテリーSOC
      obd2_batt_soc = prmA;
      *pktidx = OBD2PKT_SOC;
      break;
    case 0x042D:  // DPF PM Generated
      obd2_dpf_gen = (float)(prmA * 256 + prmB) * 100 / 65536;
      *pktidx = OBD2PKT_DPF;
      break;
    // case 0x61B1:  // 残燃料(l)
    //   obd2_fuel_liter = prmA;
    //  *pktidx = OBD2PKT_XXX;
    //   break;
    default:
      return false;
  }

  return true;
}
