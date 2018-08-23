#ifndef PC_DELWITH_HHJ
#define PC_DELWITH_HHJ

#define BAT_THRESHOLD   220 //µÍÓÚ2.2V±¨¾¯

extern void SendConfig(uint8_t com);
extern void SendBack(uint8_t com, ECHO echoCode);
extern void SendBatteryVoltage(uint8_t com);
extern void SendBackValue(uint8_t com, uint32_t d);
extern uint8_t WIFI_Dealwith(DEVICE com);
extern uint8_t WIFI_Stop(uint8_t com);
extern uint8_t PC_Stop(void);
extern uint8_t Is_PCStop(DEVICE com);

#endif






