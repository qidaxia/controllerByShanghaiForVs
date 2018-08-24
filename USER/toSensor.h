#ifndef TOSENSOR_H
#define TOSENSOR_H

extern RETCODE MotorMove(uint8_t ID, MOVECMD dir);
extern RETCODE waitLoraAck(void);
extern RETCODE MotorToPosition(uint8_t ID, uint32_t pos);
extern RETCODE MotorToZero(uint8_t ID);
extern RETCODE ReadStatus(uint8_t ID);
extern uint8_t SetXiaoChe_5V_Level(LEVELMODE levelmode);
extern uint8_t SetXiaoChe_0V_Level(void);
extern RETCODE ChangeSpeed(SPEED speed);


#endif


