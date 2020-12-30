/******************** (C) COPYRIGHT 2012 Robottime ********************
* 文件名    : lib_irq.c
* 作者       : sinbad
* 版本       : V1.0.0
* 日期       : 2012.3.28
* 所属产品 : 探索者ARM7 LPC2138主控制板
* 功能       : 中断程序
* 接口/设备: 无
********************************************************************/

#include "config.h"
#include "lib_io.h"
#include "lib_arm.h"
#include "lib_io_uart_lpc2138.h"
#include "lib_act.h"
#include "amarino.h"

int i=0;
/**********************************************************************************************************
** 函数名称 ：IRQ_UART0
** 函数功能 ：处理UART0的中断
** 入口参数 ：无
** 返  回 值 ：无
**********************************************************************************************************/
void __irq IRQ_UART0 (void)
{	
	int a;
	UART0_GetBuf();
	a = UART0_ReadBuf(); //读取接收的串口数据并赋值给变量a
	
	/* 如果使用安卓系统的amarino软件，请激活以下函数， type取值见该函数的说明 */
	/* amarino的数据处理在perception.c中执行                                                   */
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