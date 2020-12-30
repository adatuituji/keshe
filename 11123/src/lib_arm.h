#ifndef __LIB_IO_H 
#define __LIB_IO_H
extern void Initial_ARM();
extern void LedIn(uint8 Num,uint8 Color);
extern void LedOut(uint8 Num,uint8 Stat);
extern int Input(uint8 Num,uint8 Pin);
extern void Servo(uint8 Num,uint16 Ang);
extern void SendPC(uint8 data);
extern void SetReadPC(uint8 stat,uint8 priority);
extern uint8 ReadPC();
extern void SetTimer(uint32 Timer)£»
extern void TimerOpen();
extern void SetMemory(uint8 priority);
extern void SaveData(uint32 address,uint8 data);
extern uint8 LoadData(uint32 address);
extern void Motor(uint8 Num,uint16 stat);
#endif