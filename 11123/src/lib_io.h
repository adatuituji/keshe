#ifndef __LIB_IO_H 
#define __LIB_IO_H
extern uint8 GPIO_In(uint8 PortSe,uint8 PortNo,uint8 Level);
extern void GPIO_Out(uint8 PortSe,uint8 PortNo,uint8 Level);
extern uint8 AD_In(uint8 PortSe,uint8 PortNo,uint8 Min,uint8 Max);
extern void PWM(uint8 PortSe,uint8 PortNo,uint32 PW,unsigned int Tpwm) ;
extern void UART_Out(uint8 PortSe,uint8 PortNo,uint8 data,uint32 bps,uint32 xtal);
extern void UART_irq(uint8 PortSe,uint8 PortNo, uint8 stat, uint32 bps, uint32 xtal, uint32 priority);
extern uint8 UART_In(uint8 type);
extern uint8 IR_In(uint8 PortSe,uint8 PortNo);
extern void Delay(uint32 count);
extern Time_irq(uint8 PortSe,uint8 PortNo,uint8 type,uint32 count,uint32 xtal,uint8 priority);
extern void EINT_irq(uint8 PortSe,uint8 PortNo,uint8 irmod,uint8 polar,uint8 priority);
extern void IRQ_End(uint32 priority);
#define	ONE_BYTE_SUBA	1
#define TWO_BYTE_SUBA	2
#define X_ADD_8_SUBA	3

  extern     void I2cInit(uint8 PortSe,uint8 PortNo,uint8 PortNo1,uint32 Fi2c,uint8 priority);
extern uint8 I2C_ReadNByte (uint8 sla, uint32 suba_type, uint32 suba, uint32 num);
extern uint8 I2C_WriteNByte(uint8 sla, uint8 suba_type, uint32 suba, uint8 s, uint32 num);
extern void __irq IRQ_I2C(void);

//extern uint8 rcv_new;
#endif