#ifndef LIB_IRQ_H
#define LIB_IRQ_H
extern void __irq IRQ_UART0 (void);
extern void __irq IRQ_UART1 (void);
extern void __irq IRQ_Time0(void);
extern void __irq IRQ_Time1(void);
extern void __irq IRQ_Eint0(void);
extern void __irq IRQ_Eint1(void);
extern void __irq IRQ_Eint2(void);
extern void __irq IRQ_Eint3(void);
#endif