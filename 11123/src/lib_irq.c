/******************** (C) COPYRIGHT 2012 Robottime ********************
* �ļ���    : lib_irq.c
* ����       : sinbad
* �汾       : V1.0.0
* ����       : 2012.3.28
* ������Ʒ : ̽����ARM7 LPC2138�����ư�
* ����       : �жϳ���
* �ӿ�/�豸: ��
********************************************************************/

#include "config.h"
#include "lib_io.h"
#include "lib_arm.h"
#include "lib_io_uart_lpc2138.h"
#include "lib_act.h"
#include "amarino.h"

int i=0;
/**********************************************************************************************************
** �������� ��IRQ_UART0
** �������� ������UART0���ж�
** ��ڲ��� ����
** ��  �� ֵ ����
**********************************************************************************************************/
void __irq IRQ_UART0 (void)
{	
	int a;
	UART0_GetBuf();
	a = UART0_ReadBuf(); //��ȡ���յĴ������ݲ���ֵ������a
	
	/* ���ʹ�ð�׿ϵͳ��amarino������뼤�����º����� typeȡֵ���ú�����˵�� */
	/* amarino�����ݴ�����perception.c��ִ��                                                   */
	//Amarino_Process( UART0_ReadBuf(), 2 ); 
}

void __irq IRQ_UART1 (void)
{	

}

void __irq IRQ_Time0(void) 
{	
	
}

void __irq IRQ_Time1(void) 
{	
	
}

void __irq IRQ_Eint0(void)
{

}

void __irq IRQ_Eint1(void)
{

}

void __irq IRQ_Eint2(void)
{

}

void __irq IRQ_Eint3(void)
{

}