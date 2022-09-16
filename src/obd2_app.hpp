typedef enum {
    OBD2PKT_RPM,
    OBD2PKT_KMH,
    OBD2PKT_ENG_TEMP,
    OBD2PKT_FUEL_PER,
    OBD2PKT_SOC,
    OBD2PKT_DPF,
    //-----------------
    OBD2PKT_NUM,
} OBD2PKT_enum;

extern int obd2_eng_temp;
extern int obd2_rpm;
extern int obd2_kmh;
extern float obd2_fuel_per;
// extern int obd2_fuel_liter;
extern int obd2_batt_soc;
extern float obd2_dpf_gen;

void obd2_MakeStandardPacket(uint8_t *buff, uint16_t PIDcode);
void obd2_MakeCustomPacket(uint8_t *buff, uint16_t PIDcode);
bool obd2_ChkResponse(uint8_t *buff, uint16_t *PIDcode, uint8_t *prmA, uint8_t *prmB);
bool obd2_AnalysisPacket(uint16_t PIDcode, uint8_t prmA, uint8_t prmB, uint8_t *pktidx);
