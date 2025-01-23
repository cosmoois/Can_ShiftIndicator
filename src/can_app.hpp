bool can_init();
void can_onReceive(int packetSize);
bool can_getPacket(uint8_t *buff);
void can_sndPacket(uint8_t *buff);
