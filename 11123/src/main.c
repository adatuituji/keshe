#include "config.h"
#include "lib_io.h"
#include "lib_arm.h"
int main(void)
{
int s;
int i;
Initial_ARM();
while(1)
{
s=0;
for(i=0;i<3;i++)
{
s=s|(Input(i+1,1)<<i);
if(Input(i+1,1)==1) LedIn(i+1,1);
else LedIn(i+1,0);
}
switch(s)
{
case 0x01 : {Servo(1,160);Servo(2,100);}break;
case 0x02 : {Servo(1,20);Servo(2,100);}break;
case 0x04 : {Servo(1,90);Servo(2,180);}break;
case 0x07 : Act_Stop();break;
default:;break;
}
 }
return(1);
}