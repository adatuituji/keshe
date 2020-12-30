#ifndef __AMARINO_H 
#define __AMARINO_H

extern int Amarino_Process(int i, int type);
extern void Amarino_Orientation();
extern int Amarino_GetOrientation(int i);
extern int Amarino_Compass(int i,int j,int k); 
extern int Filter_AntiPulse(int channel, int new_data);

#endif