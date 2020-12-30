/******************** (C) COPYRIGHT 2012 Robottime ********************
* 文件名    : lib_io.c
* 作者       : sinbad
* 版本       : V1.0.0
* 日期       : 2012.3.7
* 所属产品 : ARM7系列芯片LPC2138
* 功能       : LPC2138的常用外设输入输出函数，如GPIO、PWM、I2C等
********************************************************************/

#include "config.h" 
#include "lib_irq.h"

/* ********************定义用于和I2C中断传递信息的全局变量******************/
uint8 s;
volatile uint8 	I2C_sla;	  //I2C器件从地址 				
volatile uint32	I2C_suba;     //	I2C器件内部子地址 				
volatile uint8 	I2C_suba_num; //I2C子地址字节数										
volatile uint8 	I2C_buf;      //I2C数据存储
volatile uint32 I2C_num;	  //要读取/写入的数据个数 			
volatile uint8 	I2C_end;	  //I2C总线结束标志：结束总线是置1 	
volatile uint8 	I2C_suba_en;  /* 子地址控制。
							        0--子地址已经处理或者不需要子地址
								1--读取操作
								2--写操作*/	
void __irq IRQ_I2C(void);		
/**********************************************************************/


/**********************************************************************************************************
** 函数名称 ：DelayNS
** 函数功能 ：长软件延时
** 入口参数 ：dly - 延时参数，值越大，延时越久
** 返  回 值 ：无
**********************************************************************************************************/
void DelayNS(uint32 dly)
{
	uint32 i;
	
	for ( ; dly>0; dly--)
		for (i=0; i<5000; i++);
}


/******************************************************************
函数名称：GPIO_In
函数功能：检测电平输入函数，检测成功返回1，失败返回0。
入口参数：PortSe：端口序列号，值为0、1；
               PortNo：端口号，值为0～31；
               Level  ：设定需检测的电平，1为高电平，0为低电平；
返  回 值：1 - 检测成功
               0 - 检测失败
备      注：ARM7 LPC2138芯片引脚范围为P0.0~P0.31，P1.16~P1.31。入口参数的
               端口序列号与端口号与引脚编号对应。详见“端口列表”。
*******************************************************************/
uint8 GPIO_In(uint8 PortSe,uint8 PortNo,uint8 Level) 
{
	if(PortSe==0)
	{
		if(PortNo<16)
		{
			PINSEL0=PINSEL0&(~(3<<(2*PortNo)));
		}
		if(PortNo>15)
		{
			PINSEL1=PINSEL1&(~(3<<(2*(PortNo-16))));
		}
		
		IO0DIR=IO0DIR|(0<<PortNo);
		
		if(Level==1)
		{
			if((IO0PIN&(1<<PortNo))!=0)
			return(1);
			else
			return(0);
		}
		else
			if((IO0PIN&(1<<PortNo))==0)
			return(1);
			else
			return(0);	
	}
	if(PortSe==1)
	{
		if(PortNo>15)
		{
			PINSEL2=PINSEL2&(~(3<<(2*(PortNo-16))));
		}
		
		IO1DIR=IO1DIR|(0<<PortNo);
		
		if(Level==1)
		{
			if((IO1PIN&(1<<PortNo))!=0)
			return(1);
			else
			return(0);
		}
		else
			if((IO1PIN&(1<<PortNo))==0)
			return(1);
			else
			return(0);
	}
}



/******************************************************************
函数名称：GPIO_Out
函数功能：控制电平输出函数。
入口参数：PortSe：端口序列号，值为0、1；
               PortNo：端口号，值为0～31；
               Level  ：设定输出的电平，1为高电平，0为低电平；
返  回 值：无
备      注：ARM7 LPC2138芯片引脚范围为P0.0~P0.31，P1.16~P1.31。入口参数的
               端口序列号与端口号与引脚编号对应。详见“端口列表”
*******************************************************************/
void GPIO_Out(uint8 PortSe,uint8 PortNo,uint8 Level) 
{
	if(PortSe==0)
	{
		if(PortNo<16)
		{
			PINSEL0=PINSEL0&(~(3<<(2*PortNo)));
		}
		if(PortNo>15)
		{
			PINSEL1=PINSEL1&(~(3<<(2*(PortNo-16))));
		}
		
		IO0DIR=IO0DIR|(1<<PortNo);
		
		if(Level==1)
		{
			IO0SET=IO0SET|(1<<PortNo);
		}
		else
			IO0CLR=IO0CLR|(1<<PortNo);
	}
	if(PortSe==1)
	{
		if(PortNo>15)
		{
			PINSEL2=PINSEL2&(~(3<<(2*(PortNo-16))));
		}
		
		IO1DIR=IO1DIR|(1<<PortNo);
		
		if(Level==1)
		{
			IO1SET=IO1SET|(1<<PortNo);
		}
		else
			IO1CLR=IO1CLR|(1<<PortNo);
	}
}



/*************************************************************************************************
函数名称：PWM
函数功能：输出脉宽调制信号
入口参数：PortSe：端口序列号，值为0、1；
               PortNo：端口号，值为0～31；
               PW     ：脉宽。|| 40 0.5ms 0度 || 20 1.0ms 45度 || 13 1.5ms 90度 || 10 2.0ms 135度  || 8 2.5ms 180度 || ；
	       Tpwm ：输出周期=Fpclk/(1000000/Tpwm)。50为20ms。
返  回 值：无
备      注：具有PWM功能的引脚为 P0.0 , P0.7 , P0.1 , P0.8 , P0.21 , P0.9
*************************************************************************************************/
void PWM(uint8 PortSe,uint8 PortNo,uint32 PW,uint32 Tpwm) 
{
	if(PortSe==0)
	{
		if(PortNo<16)
		PINSEL0=(PINSEL0&(~(0x03<<(2*PortNo))))|(0x02<<(2*PortNo));
		if(PortNo>15)
		PINSEL1=(PINSEL1&(~(0x03<<(2*(PortNo-16)))))|(0x01<<(2*(PortNo-16)));
		PWMPR    = 0x00;		    // 不分频，计数频率为Fpclk
   		PWMMCR   = 0x02;			// 设置PWMMR0匹配时复位PWMTC
   	    PWMPCR   = 0x7e00;
   	  
   	    PWMMR0   = Fpclk / ( 1000000/Tpwm); 
   	   switch(PortNo) 
   	   {
   	    	case 0:
   	    	PWMMR1   =Fpclk / ( 1000000/PW);break;
   	   	    case 7:
   	    	PWMMR2   =Fpclk / ( 1000000/PW);break;         
   	    	case 1:
   	    	PWMMR3   =Fpclk / ( 1000000/PW);break;          
   	   	    case 8:
   	    	PWMMR4   =Fpclk / ( 1000000/PW);break;           
			case 21:
			PWMMR5   =Fpclk / ( 1000000/PW);break;           
			case 9:
			PWMMR6   =Fpclk / ( 1000000/PW);break;
			default: break;
		}
		PWMLER   = 0x7f;			
    	PWMTCR   = 0x02;            
    	PWMTCR   = 0x09;            
	}
}



/*************************************************************************************************
函数名称：UART_Out
函数功能：发送一个字节串口数据
入口参数：PortSe：端口序列号，值为0、1；
               PortNo：端口号，值为0～31；
               data   ：需要发送的数据，数据类型 - uint8；
	       bps    ：波特率。建议设为 || 蓝牙通讯 - 9600 || PC通讯 - 58400 ||
	       xtal    ：晶振频率
返  回 值：无
备      注：ARM7 LPC2138有两个串行通讯端口，分别为 UART0：P0.0 P0.1 || UART1：P0.8 P0.9
*************************************************************************************************/
void UART_Out(uint8 PortSe,uint8 PortNo,uint8 data,uint32 bps,uint32 xtal) 
{uint32 Fdiv;
	
	if(PortSe==0)
	{
		if(PortNo<16)
		PINSEL0=(PINSEL0&(~(0x03<<(2*PortNo))))|(0x01<<(2*PortNo));   //UART初始化
		
		if(PortNo==0)
		{
			U0LCR=0x83;
			Fdiv=(xtal>>4)/bps;
			U0DLM=Fdiv>>8;
			U0DLL=Fdiv&0xFF;
			U0LCR=0x03;
		
			U0THR=data;
			
			
			while(U0LSR&0x20==0);
		}
		if(PortNo==8)
		{
			U1LCR=0x83;
			Fdiv=(xtal>>4)/bps;
			U1DLM=Fdiv>>8;
			U1DLL=Fdiv&0xFF;
			U1LCR=0x03;
		
			U1THR=data;
			while(U1LSR&0x20==0);
		}	
	}
}



/*************************************************************************************************
函数名称：IRQ_End
函数功能：中断处理结束。
入口参数：priority：中断优先级
返  回 值：无
*************************************************************************************************/
void IRQ_End(uint32 priority)
{
	VICVectAddr=priority;
}



/*************************************************************************************************
函数名称：UART_In
函数功能：接收一个字节串口数据
入口参数：type：串口类型 || 0 - UART0 || 1 - UART1  ||
返  回 值：接收的串口字节数据
备      注：使用前需先打开串行接收数据中断函数“UART_irq”
*************************************************************************************************/
uint8 UART_In(uint8 type)	
{
	int i;
	if(type==0)
	{
		i=U0RBR;
	}
	else
	{
		i=U1RBR;
	}
	return (i);
}



/*************************************************************************************************
函数名称：UART_irq
函数功能：打开或关闭串行接收数据中断模式
入口参数：PortSe ：端口序列号，值为0、1；
               PortNo ：端口号，值为0～31；
               stat     ：串行中断状态， 0 - 关闭，1- 开启；
	       bps     ：波特率。建议设为 || 蓝牙通讯 - 9600 || PC通讯 - 58400 ||
	       xtal     ：晶振频率
               priority：中断优先级
返  回 值：无
备      注：ARM7 LPC2138有两个串行通讯端口，分别为 UART0：P0.0 P0.1 || UART1：P0.8 P0.9
*************************************************************************************************/
void UART_irq(uint8 PortSe,uint8 PortNo, uint8 stat, uint32 bps, uint32 xtal, uint32 priority) 
{  uint32  Fdiv;
	if(stat==1)
	{
		if(PortSe==0)
		{
			if(PortNo<16)
			PINSEL0=(PINSEL0&(~(0x03<<(2*PortNo))))|(0x01<<(2*PortNo));   //UART初始化
		
			if(PortNo==1)
			{
				U0LCR=0x83;                        //除数使能
				Fdiv=(xtal>>4)/bps;                //波特率设置
				U0DLM=Fdiv>>8;
				U0DLL=Fdiv&0xFF;
				U0LCR=0x03;							//8位数据位
			
			
				
				U0FCR = 0x01;						// 使能FIFO，并设置触发点为8字节
				U0IER = 0x01;						// 允许RBR中断，即接收中断
				IRQEnable();
				VICIntSelect = 0;			// 设置所有的通道为IRQ中断
				switch(priority){
				case 0:VICVectCntl0 = 0x20 | 0x06;VICVectAddr0 = (uint32)IRQ_UART0;break;
				case 1:VICVectCntl1 = 0x20 | 0x06;VICVectAddr1 = (uint32)IRQ_UART0;break;
				case 2:VICVectCntl2 = 0x20 | 0x06;VICVectAddr2 = (uint32)IRQ_UART0;break;
				case 3:VICVectCntl3 = 0x20 | 0x06;VICVectAddr3 = (uint32)IRQ_UART0;break;
				case 4:VICVectCntl4 = 0x20 | 0x06;VICVectAddr4 = (uint32)IRQ_UART0;break;
				case 5:VICVectCntl5 = 0x20 | 0x06;VICVectAddr5 = (uint32)IRQ_UART0;break;
				case 6:VICVectCntl6 = 0x20 | 0x06;VICVectAddr6 = (uint32)IRQ_UART0;break;
				case 7:VICVectCntl7 = 0x20 | 0x06;VICVectAddr7 = (uint32)IRQ_UART0;break;
				}

				VICIntEnable = 1 << 0x06;			// 使能UART0中断
			}
		
			if(PortNo==9)
			{
				U1LCR=0x83;
				Fdiv=(xtal>>4)/bps;
				U1DLM=Fdiv>>8;
				U1DLL=Fdiv&0xFF;
				U1LCR=0x03;
		

				
				U1FCR = 0x01;						// 使能FIFO，并设置触发点为8字节
				U1IER = 0x01;						// 允许RBR中断，即接收中断
				IRQEnable();
				VICIntSelect = 0;			// 设置所有的通道为IRQ中断
					switch(priority){
				case 0:VICVectCntl0 = 0x20 | 0x07;VICVectAddr0 = (uint32)IRQ_UART1;break;
				case 1:VICVectCntl1 = 0x20 | 0x07;VICVectAddr1 = (uint32)IRQ_UART1;break;
				case 2:VICVectCntl2 = 0x20 | 0x07;VICVectAddr2 = (uint32)IRQ_UART1;break;
				case 3:VICVectCntl3 = 0x20 | 0x07;VICVectAddr3 = (uint32)IRQ_UART1;break;
				case 4:VICVectCntl4 = 0x20 | 0x07;VICVectAddr4 = (uint32)IRQ_UART1;break;
				case 5:VICVectCntl5 = 0x20 | 0x07;VICVectAddr5 = (uint32)IRQ_UART1;break;
				case 6:VICVectCntl6 = 0x20 | 0x07;VICVectAddr6 = (uint32)IRQ_UART1;break;
				case 7:VICVectCntl7 = 0x20 | 0x07;VICVectAddr7 = (uint32)IRQ_UART1;break;
				}

				VICIntEnable = 1 << 0x07;			// 使能UART0中断
			}	
		}
	}	
	
	else
		VICIntEnClr=1 << 0x06|1 << 0x07;	
}


/*************************************************************************************************
函数名称：Delay
函数功能：精确延时函数，产生x毫秒的数据
入口参数：count：毫秒
返  回 值：无
*************************************************************************************************/
void Delay(uint32 count)
{
	while(count--)
	{
		T0TC=0;
		T0PR=0;
		T0MCR=0x03;
		T0MR0=Fpclk/1000;
		T0TCR=0x01;
		while((T0IR&0x01)==0);
		T0IR=0x01;
	}	
}



/********************************************************************************************************
函数名称：Time_irq
函数功能：打开或关闭定时中断模式
入口参数：PortSe ：端口序列号，值为0、1；
               PortNo ：端口号，值为0～31；
               type    ：定时器类型；
	       count  ：产生一次定时中断的时间；
	       xtal     ：晶振频率
               priority：中断优先级
返  回 值：无
********************************************************************************************************/
void Time_irq(uint8 PortSe,uint8 PortNo,uint8 type,uint32 count,uint32 xtal,uint8 priority)
{
	if(count>0)
	{
		if(type==0)
		{
			T0TC=0;
			T0PR=0;								/*设置定时器0分频为100分频，得110592Hz*/
			T0MCR=0x03;							/*匹配通道0匹配中断并复位T0TC*/
			T0MR0=count*(xtal/1000);						/*time*/
			T0TCR=0x03;
			T0TCR=0x01;
			IRQEnable();	
			VICIntSelect=0x00;					/*所有中断通道设置为IRQ中断*/
			switch(priority){
				case 0:VICVectCntl0 = 0x20 | 0x04;VICVectAddr0 = (uint32)IRQ_Time0;break;
				case 1:VICVectCntl1 = 0x20 | 0x04;VICVectAddr1 = (uint32)IRQ_Time0;break;
				case 2:VICVectCntl2 = 0x20 | 0x04;VICVectAddr2 = (uint32)IRQ_Time0;break;
				case 3:VICVectCntl3 = 0x20 | 0x04;VICVectAddr3 = (uint32)IRQ_Time0;break;
				case 4:VICVectCntl4 = 0x20 | 0x04;VICVectAddr4 = (uint32)IRQ_Time0;break;
				case 5:VICVectCntl5 = 0x20 | 0x04;VICVectAddr5 = (uint32)IRQ_Time0;break;
				case 6:VICVectCntl6 = 0x20 | 0x04;VICVectAddr6 = (uint32)IRQ_Time0;break;
				case 7:VICVectCntl7 = 0x20 | 0x04;VICVectAddr7 = (uint32)IRQ_Time0;break;
				}

			VICIntEnable |=0x0010;				/*使能定时器中断*/
		}
		if(type==1)
		{
			T1TC=0;
			T1PR=0;								/*设置定时器1分频为100分频，得110592Hz*/
			T1MCR=0x03;							/*匹配通道0匹配中断并复位T0TC*/
			T1MR0=count*(xtal/1000);						/*time*/
			T1TCR=0x03;
			T1TCR=0x01;
			IRQEnable();	
			VICIntSelect=0x00;					/*所有中断通道设置为IRQ中断*/
			switch(priority){
				case 0:VICVectCntl0 = 0x20 | 0x05;VICVectAddr0 = (uint32)IRQ_Time1;break;
				case 1:VICVectCntl1 = 0x20 | 0x05;VICVectAddr1 = (uint32)IRQ_Time1;break;
				case 2:VICVectCntl2 = 0x20 | 0x05;VICVectAddr2 = (uint32)IRQ_Time1;break;
				case 3:VICVectCntl3 = 0x20 | 0x05;VICVectAddr3 = (uint32)IRQ_Time1;break;
				case 4:VICVectCntl4 = 0x20 | 0x05;VICVectAddr4 = (uint32)IRQ_Time1;break;
				case 5:VICVectCntl5 = 0x20 | 0x05;VICVectAddr5 = (uint32)IRQ_Time1;break;
				case 6:VICVectCntl6 = 0x20 | 0x05;VICVectAddr6 = (uint32)IRQ_Time1;break;
				case 7:VICVectCntl7 = 0x20 | 0x05;VICVectAddr7 = (uint32)IRQ_Time1;break;
				}

			VICIntEnable |=0x0020;				/*使能定时器中断*/
		}
	}
	else
		VICIntEnClr=1 << 0x04|1 << 0x05;
}
/********************************************************************************************************************/



/*************************************************************************************************
函数名称：AD_In
函数功能：读取模拟量，返回AD转换后的数据
入口参数：PortSe ：端口序列号，值为0、1；
               PortNo ：端口号，值为0～31；
               Min     ：数据转换范围的下限值
	       Max    ：数据转换范围的上限值
返  回 值：值域（Min，Max）
*************************************************************************************************/
uint16 AD_In(uint8 PortSe,uint8 PortNo,uint8 Min,uint16 Max) 
{
	uint32 ADC_Data;
	uint8 set;
	if(PortSe==0)
	{
		if(PortNo<16)
		{
			PINSEL0=(PINSEL0&(~(0x03<<(2*PortNo))))|(0x03<<(2*PortNo));
		}	
		if(PortNo>15)
		{
			PINSEL1=(PINSEL1&(~(0x03<<(2*(PortNo-16)))))|(0x01<<(2*(PortNo-16)));
		}		
		switch(PortNo)
		{
			case 27:set=0;break;
			case 28:set=1;break;
			case 29:set=2;break;
			case 30:set=3;break;
			case 25:set=4;break;
			case 26:set=5;break;
			case 4 :set=6;break;
			case 5 :set=7;break;
			default:break;
		}
		AD0CR = (1 << set)						|	// SEL=8,选择通道3
				((Fpclk / 1000000 - 1) << 8)	|	// CLKDIV=Fpclk/1000000-1,转换时钟为1MHz
				(0 << 16)						|	// BURST=0,软件控制转换操作
				(0 << 17)						|	// CLKS=0, 使用11clock转换
				(1 << 21)						|  	// PDN=1,正常工作模式
				(0 << 22)						|  	// TEST1:0=00,正常工作模式
				(1 << 24);

		ADC_Data = AD0DR;		// 读取ADC结果，并清除DONE标志位
		
		AD0CR |= 1 << 24;					// 进行第一次转换
		while ((ADDR & 0x80000000) == 0);	// 等待转换结束
		AD0CR |= 1 << 24;					// 再次启动转换
		while ((AD0DR & 0x80000000) == 0);	// 等待转换结束
		ADC_Data = AD0DR;					// 读取ADC结果
		ADC_Data = (ADC_Data >> 6) & 0x3ff;
		ADC_Data = Min+((Max-Min)*ADC_Data)/1024;
		return (ADC_Data);
	}
	else return(0);
}




/*************************************************************************************************
函数名称：EINT_irq
函数功能：打开或关闭外部中断模式，该版本只能进行边沿控制
入口参数：PortSe ：端口序列号，值为0、1；
               PortNo ：端口号，值为0～31；
               irmod  ：中断模式， 0 - 电平，1- 边沿；
	       polar   ：极性， 0 - 低电平或下降沿，1 - 高电平或上升沿；
               priority：中断优先级
返  回 值：无
*************************************************************************************************/
void EINT_irq(uint8 PortSe,uint8 PortNo,uint8 irmod,uint8 polar, uint8 priority)     
{
	uint8 eint=0;
	if(PortSe==0)
	{
		switch(PortNo)
		{
			case 1 :PINSEL0=(PINSEL0&(~(0x03<<(2*PortNo))))|(0x03<<(2*PortNo));eint=1;break;
			case 3 :PINSEL0=(PINSEL0&(~(0x03<<(2*PortNo))))|(0x03<<(2*PortNo));eint=2;break;
			case 7 :PINSEL0=(PINSEL0&(~(0x03<<(2*PortNo))))|(0x03<<(2*PortNo));eint=3;break;
			case 9 :PINSEL0=(PINSEL0&(~(0x03<<(2*PortNo))))|(0x03<<(2*PortNo));eint=4;break;
			case 14:PINSEL0=(PINSEL0&(~(0x03<<(2*PortNo))))|(0x02<<(2*PortNo));eint=2;break;
			case 15:PINSEL0=(PINSEL0&(~(0x03<<(2*PortNo))))|(0x02<<(2*PortNo));eint=3;break;
			case 16:PINSEL1=(PINSEL1&(~(0x03<<(2*(PortNo-16)))))|(0x01<<(2*(PortNo-16)));eint=1;break;
			case 20:PINSEL1=(PINSEL1&(~(0x03<<(2*(PortNo-16)))))|(0x03<<(2*(PortNo-16)));eint=4;break;
			case 30:PINSEL1=(PINSEL1&(~(0x03<<(2*(PortNo-16)))))|(0x02<<(2*(PortNo-16)));eint=4;break;
			default:break;
		}
		if(irmod==1)
		{
			if(polar==0)
			{
				switch(eint)
				{
					case 1:
					EXTMODE  =0x00;
					EXTPOLAR =0x00;
					IRQEnable();
					
					VICIntSelect   =0x00000000;
						switch(priority){
				case 0:VICVectCntl0 = 0x20 | 0x0e;VICVectAddr0 = (uint32)IRQ_Eint0;break;
				case 1:VICVectCntl1 = 0x20 | 0x0e;VICVectAddr1 = (uint32)IRQ_Eint0;break;
				case 2:VICVectCntl2 = 0x20 | 0x0e;VICVectAddr2 = (uint32)IRQ_Eint0;break;
				case 3:VICVectCntl3 = 0x20 | 0x0e;VICVectAddr3 = (uint32)IRQ_Eint0;break;
				case 4:VICVectCntl4 = 0x20 | 0x0e;VICVectAddr4 = (uint32)IRQ_Eint0;break;
				case 5:VICVectCntl5 = 0x20 | 0x0e;VICVectAddr5 = (uint32)IRQ_Eint0;break;
				case 6:VICVectCntl6 = 0x20 | 0x0e;VICVectAddr6 = (uint32)IRQ_Eint0;break;
				case 7:VICVectCntl7 = 0x20 | 0x0e;VICVectAddr7 = (uint32)IRQ_Eint0;break;

				}

					EXTINT         =0x01;
					VICIntEnable   = 1 << 0x0e;
					break;
					
					case 2:
					EXTMODE  =0x02;
					EXTPOLAR =0x00;
					IRQEnable();
					
					VICIntSelect   =0x00000000;
							switch(priority){
				case 0:VICVectCntl0 = 0x20 | 0x0f;VICVectAddr0 = (uint32)IRQ_Eint1;break;
				case 1:VICVectCntl1 = 0x20 | 0x0f;VICVectAddr1 = (uint32)IRQ_Eint1;break;
				case 2:VICVectCntl2 = 0x20 | 0x0f;VICVectAddr2 = (uint32)IRQ_Eint1;break;
				case 3:VICVectCntl3 = 0x20 | 0x0f;VICVectAddr3 = (uint32)IRQ_Eint1;break;
				case 4:VICVectCntl4 = 0x20 | 0x0f;VICVectAddr4 = (uint32)IRQ_Eint1;break;
				case 5:VICVectCntl5 = 0x20 | 0x0f;VICVectAddr5 = (uint32)IRQ_Eint1;break;
				case 6:VICVectCntl6 = 0x20 | 0x0f;VICVectAddr6 = (uint32)IRQ_Eint1;break;
				case 7:VICVectCntl7 = 0x20 | 0x0f;VICVectAddr7 = (uint32)IRQ_Eint1;break;
	
				}

					EXTINT         =0x02;
					VICIntEnable   = 1 << 0x0f;
					break;
					
					case 3:
					EXTMODE  =0x04;
					EXTPOLAR =0x00;
					IRQEnable();
					
					VICIntSelect   =0x00000000;
								switch(priority){
				case 0:VICVectCntl0 = 0x20 | 0x10;VICVectAddr0 = (uint32)IRQ_Eint2;break;
				case 1:VICVectCntl1 = 0x20 | 0x10;VICVectAddr1 = (uint32)IRQ_Eint2;break;
				case 2:VICVectCntl2 = 0x20 | 0x10;VICVectAddr2 = (uint32)IRQ_Eint2;break;
				case 3:VICVectCntl3 = 0x20 | 0x10;VICVectAddr3 = (uint32)IRQ_Eint2;break;
				case 4:VICVectCntl4 = 0x20 | 0x10;VICVectAddr4 = (uint32)IRQ_Eint2;break;
				case 5:VICVectCntl5 = 0x20 | 0x10;VICVectAddr5 = (uint32)IRQ_Eint2;break;
				case 6:VICVectCntl6 = 0x20 | 0x10;VICVectAddr6 = (uint32)IRQ_Eint2;break;
				case 7:VICVectCntl7 = 0x20 | 0x10;VICVectAddr7 = (uint32)IRQ_Eint2;break;
				
				}

					EXTINT         =0x04;
					VICIntEnable   = 1 << 0x10;
					break;
					
					case 4:
					EXTMODE  =0x08;
					EXTPOLAR =0x00;
					IRQEnable();
					
					VICIntSelect   =0x00000000;
									switch(priority){
				case 0:VICVectCntl0 = 0x20 | 0x11;VICVectAddr0 = (uint32)IRQ_Eint3;break;
				case 1:VICVectCntl1 = 0x20 | 0x11;VICVectAddr1 = (uint32)IRQ_Eint3;break;
				case 2:VICVectCntl2 = 0x20 | 0x11;VICVectAddr2 = (uint32)IRQ_Eint3;break;
				case 3:VICVectCntl3 = 0x20 | 0x11;VICVectAddr3 = (uint32)IRQ_Eint3;break;
				case 4:VICVectCntl4 = 0x20 | 0x11;VICVectAddr4 = (uint32)IRQ_Eint3;break;
				case 5:VICVectCntl5 = 0x20 | 0x11;VICVectAddr5 = (uint32)IRQ_Eint3;break;
				case 6:VICVectCntl6 = 0x20 | 0x11;VICVectAddr6 = (uint32)IRQ_Eint3;break;
				case 7:VICVectCntl7 = 0x20 | 0x11;VICVectAddr7 = (uint32)IRQ_Eint3;break;
				
				}

					EXTINT         =0x08;
					VICIntEnable   = 1 << 0x11;
					break;
					
					default:break;
				}
			}
			if(polar==1)
			{
				switch(eint)
				{
					case 1:
					EXTMODE  =0x01;
					EXTPOLAR =0x01;
					IRQEnable();
					
					VICIntSelect   =0x00000000;
										switch(priority){
				case 0:VICVectCntl0 = 0x20 | 0x10;VICVectAddr0 = (uint32)IRQ_Eint3;break;
				case 1:VICVectCntl1 = 0x20 | 0x10;VICVectAddr1 = (uint32)IRQ_Eint3;break;
				case 2:VICVectCntl2 = 0x20 | 0x10;VICVectAddr2 = (uint32)IRQ_Eint3;break;
				case 3:VICVectCntl3 = 0x20 | 0x10;VICVectAddr3 = (uint32)IRQ_Eint3;break;
				case 4:VICVectCntl4 = 0x20 | 0x10;VICVectAddr4 = (uint32)IRQ_Eint3;break;
				case 5:VICVectCntl5 = 0x20 | 0x10;VICVectAddr5 = (uint32)IRQ_Eint3;break;
				case 6:VICVectCntl6 = 0x20 | 0x10;VICVectAddr6 = (uint32)IRQ_Eint3;break;
				case 7:VICVectCntl7 = 0x20 | 0x10;VICVectAddr7 = (uint32)IRQ_Eint3;break;
			
				}

					EXTINT         =0x01;
					VICIntEnable   = 1 << 0x0e;
					break;
					
					case 2:
					EXTMODE  =0x02;
					EXTPOLAR =0x02;
					IRQEnable();
					
					VICIntSelect   =0x00000000;
										switch(priority){
				case 0:VICVectCntl0 = 0x20 | 0x10;VICVectAddr0 = (uint32)IRQ_Eint3;break;
				case 1:VICVectCntl1 = 0x20 | 0x10;VICVectAddr1 = (uint32)IRQ_Eint3;break;
				case 2:VICVectCntl2 = 0x20 | 0x10;VICVectAddr2 = (uint32)IRQ_Eint3;break;
				case 3:VICVectCntl3 = 0x20 | 0x10;VICVectAddr3 = (uint32)IRQ_Eint3;break;
				case 4:VICVectCntl4 = 0x20 | 0x10;VICVectAddr4 = (uint32)IRQ_Eint3;break;
				case 5:VICVectCntl5 = 0x20 | 0x10;VICVectAddr5 = (uint32)IRQ_Eint3;break;
				case 6:VICVectCntl6 = 0x20 | 0x10;VICVectAddr6 = (uint32)IRQ_Eint3;break;
				case 7:VICVectCntl7 = 0x20 | 0x10;VICVectAddr7 = (uint32)IRQ_Eint3;break;
			
				}

					EXTINT         =0x02;
					VICIntEnable   = 1 << 0x0f;
					break;
					
					case 3:
					EXTMODE  =0x04;
					EXTPOLAR =0x04;
					IRQEnable();
					
					VICIntSelect   =0x00000000;
										switch(priority){
				case 0:VICVectCntl0 = 0x20 | 0x10;VICVectAddr0 = (uint32)IRQ_Eint2;break;
				case 1:VICVectCntl1 = 0x20 | 0x10;VICVectAddr1 = (uint32)IRQ_Eint2;break;
				case 2:VICVectCntl2 = 0x20 | 0x10;VICVectAddr2 = (uint32)IRQ_Eint2;break;
				case 3:VICVectCntl3 = 0x20 | 0x10;VICVectAddr3 = (uint32)IRQ_Eint2;break;
				case 4:VICVectCntl4 = 0x20 | 0x10;VICVectAddr4 = (uint32)IRQ_Eint2;break;
				case 5:VICVectCntl5 = 0x20 | 0x10;VICVectAddr5 = (uint32)IRQ_Eint2;break;
				case 6:VICVectCntl6 = 0x20 | 0x10;VICVectAddr6 = (uint32)IRQ_Eint2;break;
				case 7:VICVectCntl7 = 0x20 | 0x10;VICVectAddr7 = (uint32)IRQ_Eint2;break;
				
				}

					EXTINT         =0x04;
					VICIntEnable   = 1 << 0x10;
					break;
					
					case 4:
					EXTMODE  =0x08;
					EXTPOLAR =0x08;
					IRQEnable();
					
					VICIntSelect   =0x00000000;
										switch(priority){
				case 0:VICVectCntl0 = 0x20 | 0x11;VICVectAddr0 = (uint32)IRQ_Eint3;break;
				case 1:VICVectCntl1 = 0x20 | 0x11;VICVectAddr1 = (uint32)IRQ_Eint3;break;
				case 2:VICVectCntl2 = 0x20 | 0x11;VICVectAddr2 = (uint32)IRQ_Eint3;break;
				case 3:VICVectCntl3 = 0x20 | 0x11;VICVectAddr3 = (uint32)IRQ_Eint3;break;
				case 4:VICVectCntl4 = 0x20 | 0x11;VICVectAddr4 = (uint32)IRQ_Eint3;break;
				case 5:VICVectCntl5 = 0x20 | 0x11;VICVectAddr5 = (uint32)IRQ_Eint3;break;
				case 6:VICVectCntl6 = 0x20 | 0x11;VICVectAddr6 = (uint32)IRQ_Eint3;break;
				case 7:VICVectCntl7 = 0x20 | 0x11;VICVectAddr7 = (uint32)IRQ_Eint3;break;
			
				}

					EXTINT         =0x08;
					VICIntEnable   = 1 << 0x11;
					break;
					
					default:break;
				}
			}
		}
	}
}



/*************************************************************************************************
函数名称：DA_Out
函数功能：DA转换后输出模拟量
入口参数：PortSe ：端口序列号，值为0、1；
               PortNo ：端口号，值为0～31；
	       DaData：输出模拟电压，值域为（0，,1024）；
返  回 值：无
*************************************************************************************************/
void DA_Out(uint8 PortSe,uint8 PortNo,uint16 DaData) 
{
	if(PortSe==0)
	{
		if(PortNo>15)
		 {
			PINSEL1=(PINSEL1&(~(3<<(2*(PortNo-16)))))|(2<<(2*(PortNo-16)));
		 }

		  DACR=(DaData<<6);
	}

}



/*************************************************************************************************
函数名称：I2cInit
函数功能：I2C初始化
入口参数：PortSe   ：端口序列号，值为0、1；
               PortNo   ：端口号，值为0～31；
               PortNo1 ：端口号，值为0～31；
	       Fi2c       ：I2C总线频率(最大400K)//100k=100000，即传输速率
               priority  ：中断优先级
返  回 值：无
*************************************************************************************************/
void I2cInit(uint8 PortSe,uint8 PortNo,uint8 PortNo1,uint32 Fi2c,uint8 priority)
{

   				
   	if(PortSe==0){
   	if((PortNo==2)&&(PortNo1==3)){			
	PINSEL0 = (PINSEL0 & (~0xF0)) | 0x50; 	// 不影响其它管脚连接
		VICIntSelect = 0x00000000;			/* 设置所有通道为IRQ中断 */
					
					switch(priority){
				case 0:VICVectCntl0 = 0x20 | 0x09;VICVectAddr0 = (uint32)IRQ_I2C;break;
				case 1:VICVectCntl1 = 0x20 | 0x09;VICVectAddr1 = (uint32)IRQ_I2C;break;
				case 2:VICVectCntl2 = 0x20 | 0x09;VICVectAddr2 = (uint32)IRQ_I2C;break;
				case 3:VICVectCntl3 = 0x20 | 0x09;VICVectAddr3 = (uint32)IRQ_I2C;break;
				case 4:VICVectCntl4 = 0x20 | 0x09;VICVectAddr4 = (uint32)IRQ_I2C;break;
				case 5:VICVectCntl5 = 0x20 | 0x09;VICVectAddr5 = (uint32)IRQ_I2C;break;
				case 6:VICVectCntl6 = 0x20 | 0x09;VICVectAddr6 = (uint32)IRQ_I2C;break;
				case 7:VICVectCntl7 = 0x20 | 0x09;VICVectAddr7 = (uint32)IRQ_I2C;break;
			
				}

	VICIntEnable = (1 << 9);
	}
	else if((PortNo==11)&&(PortNo1==14)){
	PINSEL0 = (PINSEL0 & (~0x30c00000)) | 0x30c00000; 	// 不影响其它管脚连接
	VICIntSelect = 0x00000000;							/* 设置所有通道为IRQ中断 	*/
			switch(priority){
				case 0:VICVectCntl0 = 0x20 | 0x19;VICVectAddr0 = (uint32)IRQ_I2C;break;
				case 1:VICVectCntl1 = 0x20 | 0x19;VICVectAddr1 = (uint32)IRQ_I2C;break;
				case 2:VICVectCntl2 = 0x20 | 0x19;VICVectAddr2 = (uint32)IRQ_I2C;break;
				case 3:VICVectCntl3 = 0x20 | 0x19;VICVectAddr3 = (uint32)IRQ_I2C;break;
				case 4:VICVectCntl4 = 0x20 | 0x19;VICVectAddr4 = (uint32)IRQ_I2C;break;
				case 5:VICVectCntl5 = 0x20 | 0x19;VICVectAddr5 = (uint32)IRQ_I2C;break;
				case 6:VICVectCntl6 = 0x20 | 0x19;VICVectAddr6 = (uint32)IRQ_I2C;break;
				case 7:VICVectCntl7 = 0x20 | 0x19;VICVectAddr7 = (uint32)IRQ_I2C;break;
			
				}

	VICIntEnable = (1 << 19);
	}
	}
	I2SCLH = (Fpclk/Fi2c + 1) / 2;						/* 设定I2C时钟 						*/
	I2SCLL = (Fpclk/Fi2c)/2;
	I2CONCLR = 0x2C;
	I2CONSET = 0x40;									/* 使能主I2C 						*/
	IRQEnable();
}


/**********************************************************************************************************
函数名称 ：I2C_ReadNByte
函数功能 ：从有子地址器件任意地址开始读取N字节数据
入口参数 ：sla	       ：器件从地址
		suba_type：子地址结构	1－单字节地址	2－8+X结构	2－双字节地址
		suba        ：器件子地址
		s             ：数据接收缓冲区指针
		num        ：读取的个数
返  回 值 ：TRUE  - 操作成功
		FALSE - 操作失败
**********************************************************************************************************/
uint8 I2C_ReadNByte (uint8 sla, uint32 suba_type, uint32 suba, uint8 num)
{
	if (num > 0)	/* 判断num个数的合法性 */
	{	/* 参数设置 */
		if (suba_type == 1)
		{	/* 子地址为单字节 */
			I2C_sla     	= sla + 1;							/* 读器件的从地址，R=1 	*/
			I2C_suba    	= suba;								/* 器件子地址 			*/
			I2C_suba_num	= 1;								/* 器件子地址为1字节 	*/
		}
		if (suba_type == 2)
		{	/* 子地址为2字节 */
			I2C_sla     	= sla + 1;							/* 读器件的从地址，R=1 	*/
			I2C_suba   	 	= suba;								/* 器件子地址 			*/
			I2C_suba_num	= 2;								/* 器件子地址为2字节 	*/
		}
		if (suba_type == 3)
		{	/* 子地址结构为8+X*/
			I2C_sla			= sla + ((suba >> 7 )& 0x0e) + 1;	/* 读器件的从地址，R=1	*/
			I2C_suba		= suba & 0x0ff;						/* 器件子地址	 		*/
			I2C_suba_num	= 1;								/* 器件子地址为8+x	 	*/
		}
		//I2C_buf     = s;										/* 数据接收缓冲区指针 	*/
	   I2C_num     = num;										/* 要读取的个数 		*/
		I2C_suba_en = 1;										/* 有子地址读 			*/
		I2C_end     = 0;
		
		/* 清除STA,SI,AA标志位 */
		I2CONCLR = 	(1 << 2)|	/* AA 		*/
					(1 << 3)|	/* SI 		*/
					(1 << 5);	/* STA 		*/
		
		/* 置位STA,启动I2C总线 */
		I2CONSET = 	(1 << 5)|	/* STA 		*/
					(1 << 6);	/* I2CEN 	*/
		
		/* 等待I2C操作完成 */
		while (I2C_end == 0)
		{	}
		if (I2C_end == 1)
			return (I2C_buf);
			//return (TRUE);
		else
			return (FALSE);			
	}
	return (FALSE);
}


/**********************************************************************************************************
函数名称 ：I2C_WriteNByte
函数功能 ：向有子地址器件写入N字节数据
入口参数 ：sla	       ：器件从地址0xAo
		suba_type：子地址结构	1－单字节地址	3－8+X结构	2－双字节地址//2
		suba	       ：器件内部物理地址//
		*s	       ：将要写入的数据的指针
		 num	       ：将要写入的数据的个数
返  回 值 ： TRUE  - 操作成功
		 FALSE - 操作失败
**********************************************************************************************************/
uint8 I2C_WriteNByte(uint8 sla, uint8 suba_type, uint32 suba, uint8 s, uint32 num)
{
	if (num > 0)/* 如果读取的个数为0，则返回错误 */
	{	/* 设置参数 */	
		if (suba_type == 1)
		{	/* 子地址为单字节 */
			I2C_sla     	= sla;								/* 读器件的从地址	 	*/
			I2C_suba    	= suba;								/* 器件子地址 			*/
			I2C_suba_num	= 1;								/* 器件子地址为1字节 	*/
		}
		if (suba_type == 2)
		{	/* 子地址为2字节 */
			I2C_sla     	= sla;								/* 读器件的从地址 		*/
			I2C_suba   	 	= suba;								/* 器件子地址 			*/
			I2C_suba_num	= 2;								/* 器件子地址为2字节 	*/
		}
		if (suba_type == 3)
		{	/* 子地址结构为8+X */
			I2C_sla			= sla + ((suba >> 7 )& 0x0e);		/* 读器件的从地址		*/
			I2C_suba		= suba & 0x0ff;						/* 器件子地址			*/
			I2C_suba_num	= 1;								/* 器件子地址为8+X	 	*/
		}

	    I2C_buf     = s;										/* 数据 				*/
		I2C_num     = num;										/* 数据个数 			*/
		I2C_suba_en = 2;										/* 有子地址，写操作 	*/
		I2C_end     = 0;
		
		/* 清除STA,SI,AA标志位 */
		I2CONCLR = 	(1 << 2)|	/* AA 	*/
					(1 << 3)|	/* SI 	*/
					(1 << 5);	/* STA 	*/
		
		/* 置位STA,启动I2C总线 */
		I2CONSET = 	(1 << 5)|	/* STA 	*/
					(1 << 6);	/* I2CEN*/
		
		/* 等待I2C操作完成 */
		while (I2C_end == 0) 
		{	}
		if (I2C_end == 1)
	
		    { DelayNS(10);
			return (TRUE);}
		else
			return (FALSE);	
	}
	return (FALSE);
}


/**********************************************************************************************************
函数名称 ：__irq IRQ_I2C()
函数名次 ：硬件I2C中断服务程序。 
入口参数 ：无
返  回 值 ：无
备      注 ：注意处理子地址为2字节的情况。 
**********************************************************************************************************/
void __irq IRQ_I2C(void)
{	/* 读取I2C状态寄存器I2DAT */
	/* 按照全局变量的设置进行操作及设置软件标志 */
	/* 清除中断逻辑,中断返回 */
	
	switch (I2STAT & 0xF8)
	{	/* 根据状态码进行相应的处理 */
		case 0x08:	/* 已发送起始条件 */				/* 主发送和主接收都有 		*/
			/* 装入SLA+W或者SLA+R */
		 	if(I2C_suba_en == 1)/* SLA+R */				/* 指定子地址读 			*/
		 	{	I2DAT = I2C_sla & 0xFE; 				/* 先写入地址 				*/
		 	}
            else	/* SLA+W */
            {  	I2DAT = I2C_sla;        				/* 否则直接发送从机地址 	*/
            }
            /* 清零SI位 */
            I2CONCLR =	(1 << 3)|						/* SI 						*/
            			(1 << 5);						/* STA 						*/
            break;
            
       	case 0x10:	/*已发送重复起始条件 */ 			/* 主发送和主接收都有 		*/
       		/* 装入SLA+W或者SLA+R */
       		I2DAT = I2C_sla;							/* 重起总线后，重发从地址 	*/
       		I2CONCLR = 0x28;							/* 清零SI,STA */
       		break;

		case 0x18:
       	case 0x28:	/* 已发送I2DAT中的数据，已接收ACK */
       		if (I2C_suba_en == 0)
       		{
	       		if (I2C_num > 0)
	       		{	I2DAT = I2C_buf;
	       		//I2DAT = *I2C_buf++;
	       			I2CONCLR = 0x28;					/* 清零SI,STA 				*/
	       			I2C_num--;
	       		}
	       		else	/* 没有数据发送了 */
	       		{		/* 停止总线 */
	       		  	I2CONSET = (1 << 4);				/* STO 						*/
	       			I2CONCLR = 0x28;					/* 清零SI,STA 				*/
	       		  	I2C_end = 1;						/* 总线已经停止 			*/
	       		}
       		}
       		
            if(I2C_suba_en == 1)	/* 若是指定地址读，则重新启动总线 				*/
            { 
            	if (I2C_suba_num == 2)
            	{	I2DAT = ((I2C_suba >> 8) & 0xff);
	       			I2CONCLR = 0x28;					/* 清零SI,STA 				*/
	       			I2C_suba_num--;
	       			break;	
	       		} 
	       		
	       		if(I2C_suba_num == 1)
	       		{	I2DAT = (I2C_suba & 0xff);
	       			I2CONCLR = 0x28;					/* 清零SI,STA 				*/
	       			I2C_suba_num--;
	       			break;	
	       		}
	       		
            	if (I2C_suba_num == 0)
            	{	I2CONSET = 0x20;
               		I2CONCLR = 0x08;
               		I2C_suba_en = 0;     				/* 子地址己处理 			*/
               		break;
               	}
            }
            
            if (I2C_suba_en == 2)/* 指定子地址写,子地址尚未指定,则发送子地址 		*/
       		{
       		 	if (I2C_suba_num > 0)
            	{	if (I2C_suba_num == 2)
            		{	I2DAT = ((I2C_suba >> 8) & 0xff);
            			I2CONCLR = 0x28;
            			I2C_suba_num--;
            			break;
            		}
            		if (I2C_suba_num == 1)
            		{	I2DAT    = (I2C_suba & 0xff);
               			I2CONCLR = 0x28;
               			I2C_suba_num--;
               			I2C_suba_en  = 0;
               			break;
               		}
               	}
             }
       		break;
       		  
       case 0x40:	/* 已发送SLA+R,已接收ACK */
       		if (I2C_num <= 1)	/* 如果是最后一个字节 */			
       		{	I2CONCLR = 1 << 2;      				/* 下次发送非应答信号 		*/
       		}
       		else
       		{ 	I2CONSET = 1 << 2;						/* 下次发送应答信号 		*/
       		}
       		I2CONCLR = 0x28;							/* 清零SI,STA 				*/
       		break;

       	case 0x20:	/* 已发送SLA+W,已接收非应答              */
       	case 0x30:	/* 已发送I2DAT中的数据，已接收非应答     */
       	case 0x38:	/* 在SLA+R/W或数据字节中丢失仲裁         */
   		case 0x48:	/* 已发送SLA+R,已接收非应答              */
         	I2CONCLR = 0x28;
            I2C_end = 0xFF; 
       		break;   				
	
		case 0x50:	/* 已接收数据字节，已返回ACK */
		       I2C_buf = I2DAT;
			//*I2C_buf++ = I2DAT;
			I2C_num--;
			if (I2C_num == 1)/* 接收最后一个字节 */
			{  	I2CONCLR = 0x2C;						/* STA,SI,AA = 0 			*/
			}
			else
			{ 
			 	I2CONSET = 0x04;						    /* AA=1 		*/
			  	I2CONCLR = 0x28;
			}
			break;
		
		case 0x58:	/* 已接收数据字节，已返回非应答 */
		//	*I2C_buf++ = I2DAT;     					/* 读取最后一字节数据 		*/
		    I2C_buf = I2DAT; 
            I2CONSET = 0x10;        					/* 结束总线 				*/
            I2CONCLR = 0x28;
            I2C_end = 1; 
            break;
            
      	default:
      		break;
	}
   VICVectAddr = 0x00;              					/* 中断处理结束 			*/
}


