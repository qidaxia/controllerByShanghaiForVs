#ifndef PLANTASK_HH_H
#define PLANTASK_HH_H
#include "design.h"

extern RETCODE waitDeviceToThere(uint32_t ID, uint32_t des);
extern void PlanTask(void);
extern uint8_t Scan_Full(void);
extern uint8_t Scan_Part(void);
extern uint8_t Scan_Row(uint8_t scan_row);
extern uint8_t Scan_Column(uint16_t scan_column);

#endif
