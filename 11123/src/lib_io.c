/******************** (C) COPYRIGHT 2012 Robottime ********************
* �ļ���    : lib_io.c
* ����       : sinbad
* �汾       : V1.0.0
* ����       : 2012.3.7
* ������Ʒ : ARM7ϵ��оƬLPC2138
* ����       : LPC2138�ĳ����������������������GPIO��PWM��I2C��
********************************************************************/

#include "config.h" 
#include "lib_irq.h"

/* ********************�������ں�I2C�жϴ�����Ϣ��ȫ�ֱ���******************/
uint8 s;
volatile uint8 	I2C_sla;	  //I2C�����ӵ�ַ 				
volatile uint32	I2C_suba;     //	I2C�����ڲ��ӵ�ַ 				
volatile uint8 	I2C_suba_num; //I2C�ӵ�ַ�ֽ���										
volatile uint8 	I2C_buf;      //I2C���ݴ洢
volatile uint32 I2C_num;	  //Ҫ��ȡ/д������ݸ��� 			
volatile uint8 	I2C_end;	  //I2C���߽�����־��������������1 	
volatile uint8 	I2C_suba_en;  /* �ӵ�ַ���ơ�
							        0--�ӵ�ַ�Ѿ�������߲���Ҫ�ӵ�ַ
								1--��ȡ����
								2--д����*/	
void __irq IRQ_I2C(void);		
/**********************************************************************/


/**********************************************************************************************************
** �������� ��DelayNS
** �������� ���������ʱ
** ��ڲ��� ��dly - ��ʱ������ֵԽ����ʱԽ��
** ��  �� ֵ ����
**********************************************************************************************************/
void DelayNS(uint32 dly)
{
	uint32 i;
	
	for ( ; dly>0; dly--)
		for (i=0; i<5000; i++);
}


/******************************************************************
�������ƣ�GPIO_In
�������ܣ�����ƽ���뺯�������ɹ�����1��ʧ�ܷ���0��
��ڲ�����PortSe���˿����кţ�ֵΪ0��1��
               PortNo���˿ںţ�ֵΪ0��31��
               Level  ���趨����ĵ�ƽ��1Ϊ�ߵ�ƽ��0Ϊ�͵�ƽ��
��  �� ֵ��1 - ���ɹ�
               0 - ���ʧ��
��      ע��ARM7 LPC2138оƬ���ŷ�ΧΪP0.0~P0.31��P1.16~P1.31����ڲ�����
               �˿����к���˿ں������ű�Ŷ�Ӧ��������˿��б���
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
�������ƣ�GPIO_Out
�������ܣ����Ƶ�ƽ���������
��ڲ�����PortSe���˿����кţ�ֵΪ0��1��
               PortNo���˿ںţ�ֵΪ0��31��
               Level  ���趨����ĵ�ƽ��1Ϊ�ߵ�ƽ��0Ϊ�͵�ƽ��
��  �� ֵ����
��      ע��ARM7 LPC2138оƬ���ŷ�ΧΪP0.0~P0.31��P1.16~P1.31����ڲ�����
               �˿����к���˿ں������ű�Ŷ�Ӧ��������˿��б�
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
�������ƣ�PWM
�������ܣ������������ź�
��ڲ�����PortSe���˿����кţ�ֵΪ0��1��
               PortNo���˿ںţ�ֵΪ0��31��
               PW     ������|| 40 0.5ms 0�� || 20 1.0ms 45�� || 13 1.5ms 90�� || 10 2.0ms 135��  || 8 2.5ms 180�� || ��
	       Tpwm ���������=Fpclk/(1000000/Tpwm)��50Ϊ20ms��
��  �� ֵ����
��      ע������PWM���ܵ�����Ϊ P0.0 , P0.7 , P0.1 , P0.8 , P0.21 , P0.9
*************************************************************************************************/
void PWM(uint8 PortSe,uint8 PortNo,uint32 PW,uint32 Tpwm) 
{
	if(PortSe==0)
	{
		if(PortNo<16)
		PINSEL0=(PINSEL0&(~(0x03<<(2*PortNo))))|(0x02<<(2*PortNo));
		if(PortNo>15)
		PINSEL1=(PINSEL1&(~(0x03<<(2*(PortNo-16)))))|(0x01<<(2*(PortNo-16)));
		PWMPR    = 0x00;		    // ����Ƶ������Ƶ��ΪFpclk
   		PWMMCR   = 0x02;			// ����PWMMR0ƥ��ʱ��λPWMTC
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
�������ƣ�UART_Out
�������ܣ�����һ���ֽڴ�������
��ڲ�����PortSe���˿����кţ�ֵΪ0��1��
               PortNo���˿ںţ�ֵΪ0��31��
               data   ����Ҫ���͵����ݣ��������� - uint8��
	       bps    �������ʡ�������Ϊ || ����ͨѶ - 9600 || PCͨѶ - 58400 ||
	       xtal    ������Ƶ��
��  �� ֵ����
��      ע��ARM7 LPC2138����������ͨѶ�˿ڣ��ֱ�Ϊ UART0��P0.0 P0.1 || UART1��P0.8 P0.9
*************************************************************************************************/
void UART_Out(uint8 PortSe,uint8 PortNo,uint8 data,uint32 bps,uint32 xtal) 
{uint32 Fdiv;
	
	if(PortSe==0)
	{
		if(PortNo<16)
		PINSEL0=(PINSEL0&(~(0x03<<(2*PortNo))))|(0x01<<(2*PortNo));   //UART��ʼ��
		
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
�������ƣ�IRQ_End
�������ܣ��жϴ��������
��ڲ�����priority���ж����ȼ�
��  �� ֵ����
*************************************************************************************************/
void IRQ_End(uint32 priority)
{
	VICVectAddr=priority;
}



/*************************************************************************************************
�������ƣ�UART_In
�������ܣ�����һ���ֽڴ�������
��ڲ�����type���������� || 0 - UART0 || 1 - UART1  ||
��  �� ֵ�����յĴ����ֽ�����
��      ע��ʹ��ǰ���ȴ򿪴��н��������жϺ�����UART_irq��
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
�������ƣ�UART_irq
�������ܣ��򿪻�رմ��н��������ж�ģʽ
��ڲ�����PortSe ���˿����кţ�ֵΪ0��1��
               PortNo ���˿ںţ�ֵΪ0��31��
               stat     �������ж�״̬�� 0 - �رգ�1- ������
	       bps     �������ʡ�������Ϊ || ����ͨѶ - 9600 || PCͨѶ - 58400 ||
	       xtal     ������Ƶ��
               priority���ж����ȼ�
��  �� ֵ����
��      ע��ARM7 LPC2138����������ͨѶ�˿ڣ��ֱ�Ϊ UART0��P0.0 P0.1 || UART1��P0.8 P0.9
*************************************************************************************************/
void UART_irq(uint8 PortSe,uint8 PortNo, uint8 stat, uint32 bps, uint32 xtal, uint32 priority) 
{  uint32  Fdiv;
	if(stat==1)
	{
		if(PortSe==0)
		{
			if(PortNo<16)
			PINSEL0=(PINSEL0&(~(0x03<<(2*PortNo))))|(0x01<<(2*PortNo));   //UART��ʼ��
		
			if(PortNo==1)
			{
				U0LCR=0x83;                        //����ʹ��
				Fdiv=(xtal>>4)/bps;                //����������
				U0DLM=Fdiv>>8;
				U0DLL=Fdiv&0xFF;
				U0LCR=0x03;							//8λ����λ
			
			
				
				U0FCR = 0x01;						// ʹ��FIFO�������ô�����Ϊ8�ֽ�
				U0IER = 0x01;						// ����RBR�жϣ��������ж�
				IRQEnable();
				VICIntSelect = 0;			// �������е�ͨ��ΪIRQ�ж�
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

				VICIntEnable = 1 << 0x06;			// ʹ��UART0�ж�
			}
		
			if(PortNo==9)
			{
				U1LCR=0x83;
				Fdiv=(xtal>>4)/bps;
				U1DLM=Fdiv>>8;
				U1DLL=Fdiv&0xFF;
				U1LCR=0x03;
		

				
				U1FCR = 0x01;						// ʹ��FIFO�������ô�����Ϊ8�ֽ�
				U1IER = 0x01;						// ����RBR�жϣ��������ж�
				IRQEnable();
				VICIntSelect = 0;			// �������е�ͨ��ΪIRQ�ж�
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

				VICIntEnable = 1 << 0x07;			// ʹ��UART0�ж�
			}	
		}
	}	
	
	else
		VICIntEnClr=1 << 0x06|1 << 0x07;	
}


/*************************************************************************************************
�������ƣ�Delay
�������ܣ���ȷ��ʱ����������x���������
��ڲ�����count������
��  �� ֵ����
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
�������ƣ�Time_irq
�������ܣ��򿪻�رն�ʱ�ж�ģʽ
��ڲ�����PortSe ���˿����кţ�ֵΪ0��1��
               PortNo ���˿ںţ�ֵΪ0��31��
               type    ����ʱ�����ͣ�
	       count  ������һ�ζ�ʱ�жϵ�ʱ�䣻
	       xtal     ������Ƶ��
               priority���ж����ȼ�
��  �� ֵ����
********************************************************************************************************/
void Time_irq(uint8 PortSe,uint8 PortNo,uint8 type,uint32 count,uint32 xtal,uint8 priority)
{
	if(count>0)
	{
		if(type==0)
		{
			T0TC=0;
			T0PR=0;								/*���ö�ʱ��0��ƵΪ100��Ƶ����110592Hz*/
			T0MCR=0x03;							/*ƥ��ͨ��0ƥ���жϲ���λT0TC*/
			T0MR0=count*(xtal/1000);						/*time*/
			T0TCR=0x03;
			T0TCR=0x01;
			IRQEnable();	
			VICIntSelect=0x00;					/*�����ж�ͨ������ΪIRQ�ж�*/
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

			VICIntEnable |=0x0010;				/*ʹ�ܶ�ʱ���ж�*/
		}
		if(type==1)
		{
			T1TC=0;
			T1PR=0;								/*���ö�ʱ��1��ƵΪ100��Ƶ����110592Hz*/
			T1MCR=0x03;							/*ƥ��ͨ��0ƥ���жϲ���λT0TC*/
			T1MR0=count*(xtal/1000);						/*time*/
			T1TCR=0x03;
			T1TCR=0x01;
			IRQEnable();	
			VICIntSelect=0x00;					/*�����ж�ͨ������ΪIRQ�ж�*/
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

			VICIntEnable |=0x0020;				/*ʹ�ܶ�ʱ���ж�*/
		}
	}
	else
		VICIntEnClr=1 << 0x04|1 << 0x05;
}
/********************************************************************************************************************/



/*************************************************************************************************
�������ƣ�AD_In
�������ܣ���ȡģ����������ADת���������
��ڲ�����PortSe ���˿����кţ�ֵΪ0��1��
               PortNo ���˿ںţ�ֵΪ0��31��
               Min     ������ת����Χ������ֵ
	       Max    ������ת����Χ������ֵ
��  �� ֵ��ֵ��Min��Max��
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
		AD0CR = (1 << set)						|	// SEL=8,ѡ��ͨ��3
				((Fpclk / 1000000 - 1) << 8)	|	// CLKDIV=Fpclk/1000000-1,ת��ʱ��Ϊ1MHz
				(0 << 16)						|	// BURST=0,�������ת������
				(0 << 17)						|	// CLKS=0, ʹ��11clockת��
				(1 << 21)						|  	// PDN=1,��������ģʽ
				(0 << 22)						|  	// TEST1:0=00,��������ģʽ
				(1 << 24);

		ADC_Data = AD0DR;		// ��ȡADC����������DONE��־λ
		
		AD0CR |= 1 << 24;					// ���е�һ��ת��
		while ((ADDR & 0x80000000) == 0);	// �ȴ�ת������
		AD0CR |= 1 << 24;					// �ٴ�����ת��
		while ((AD0DR & 0x80000000) == 0);	// �ȴ�ת������
		ADC_Data = AD0DR;					// ��ȡADC���
		ADC_Data = (ADC_Data >> 6) & 0x3ff;
		ADC_Data = Min+((Max-Min)*ADC_Data)/1024;
		return (ADC_Data);
	}
	else return(0);
}




/*************************************************************************************************
�������ƣ�EINT_irq
�������ܣ��򿪻�ر��ⲿ�ж�ģʽ���ð汾ֻ�ܽ��б��ؿ���
��ڲ�����PortSe ���˿����кţ�ֵΪ0��1��
               PortNo ���˿ںţ�ֵΪ0��31��
               irmod  ���ж�ģʽ�� 0 - ��ƽ��1- ���أ�
	       polar   �����ԣ� 0 - �͵�ƽ���½��أ�1 - �ߵ�ƽ�������أ�
               priority���ж����ȼ�
��  �� ֵ����
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
�������ƣ�DA_Out
�������ܣ�DAת�������ģ����
��ڲ�����PortSe ���˿����кţ�ֵΪ0��1��
               PortNo ���˿ںţ�ֵΪ0��31��
	       DaData�����ģ���ѹ��ֵ��Ϊ��0��,1024����
��  �� ֵ����
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
�������ƣ�I2cInit
�������ܣ�I2C��ʼ��
��ڲ�����PortSe   ���˿����кţ�ֵΪ0��1��
               PortNo   ���˿ںţ�ֵΪ0��31��
               PortNo1 ���˿ںţ�ֵΪ0��31��
	       Fi2c       ��I2C����Ƶ��(���400K)//100k=100000������������
               priority  ���ж����ȼ�
��  �� ֵ����
*************************************************************************************************/
void I2cInit(uint8 PortSe,uint8 PortNo,uint8 PortNo1,uint32 Fi2c,uint8 priority)
{

   				
   	if(PortSe==0){
   	if((PortNo==2)&&(PortNo1==3)){			
	PINSEL0 = (PINSEL0 & (~0xF0)) | 0x50; 	// ��Ӱ�������ܽ�����
		VICIntSelect = 0x00000000;			/* ��������ͨ��ΪIRQ�ж� */
					
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
	PINSEL0 = (PINSEL0 & (~0x30c00000)) | 0x30c00000; 	// ��Ӱ�������ܽ�����
	VICIntSelect = 0x00000000;							/* ��������ͨ��ΪIRQ�ж� 	*/
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
	I2SCLH = (Fpclk/Fi2c + 1) / 2;						/* �趨I2Cʱ�� 						*/
	I2SCLL = (Fpclk/Fi2c)/2;
	I2CONCLR = 0x2C;
	I2CONSET = 0x40;									/* ʹ����I2C 						*/
	IRQEnable();
}


/**********************************************************************************************************
�������� ��I2C_ReadNByte
�������� �������ӵ�ַ���������ַ��ʼ��ȡN�ֽ�����
��ڲ��� ��sla	       �������ӵ�ַ
		suba_type���ӵ�ַ�ṹ	1�����ֽڵ�ַ	2��8+X�ṹ	2��˫�ֽڵ�ַ
		suba        �������ӵ�ַ
		s             �����ݽ��ջ�����ָ��
		num        ����ȡ�ĸ���
��  �� ֵ ��TRUE  - �����ɹ�
		FALSE - ����ʧ��
**********************************************************************************************************/
uint8 I2C_ReadNByte (uint8 sla, uint32 suba_type, uint32 suba, uint8 num)
{
	if (num > 0)	/* �ж�num�����ĺϷ��� */
	{	/* �������� */
		if (suba_type == 1)
		{	/* �ӵ�ַΪ���ֽ� */
			I2C_sla     	= sla + 1;							/* �������Ĵӵ�ַ��R=1 	*/
			I2C_suba    	= suba;								/* �����ӵ�ַ 			*/
			I2C_suba_num	= 1;								/* �����ӵ�ַΪ1�ֽ� 	*/
		}
		if (suba_type == 2)
		{	/* �ӵ�ַΪ2�ֽ� */
			I2C_sla     	= sla + 1;							/* �������Ĵӵ�ַ��R=1 	*/
			I2C_suba   	 	= suba;								/* �����ӵ�ַ 			*/
			I2C_suba_num	= 2;								/* �����ӵ�ַΪ2�ֽ� 	*/
		}
		if (suba_type == 3)
		{	/* �ӵ�ַ�ṹΪ8+X*/
			I2C_sla			= sla + ((suba >> 7 )& 0x0e) + 1;	/* �������Ĵӵ�ַ��R=1	*/
			I2C_suba		= suba & 0x0ff;						/* �����ӵ�ַ	 		*/
			I2C_suba_num	= 1;								/* �����ӵ�ַΪ8+x	 	*/
		}
		//I2C_buf     = s;										/* ���ݽ��ջ�����ָ�� 	*/
	   I2C_num     = num;										/* Ҫ��ȡ�ĸ��� 		*/
		I2C_suba_en = 1;										/* ���ӵ�ַ�� 			*/
		I2C_end     = 0;
		
		/* ���STA,SI,AA��־λ */
		I2CONCLR = 	(1 << 2)|	/* AA 		*/
					(1 << 3)|	/* SI 		*/
					(1 << 5);	/* STA 		*/
		
		/* ��λSTA,����I2C���� */
		I2CONSET = 	(1 << 5)|	/* STA 		*/
					(1 << 6);	/* I2CEN 	*/
		
		/* �ȴ�I2C������� */
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
�������� ��I2C_WriteNByte
�������� �������ӵ�ַ����д��N�ֽ�����
��ڲ��� ��sla	       �������ӵ�ַ0xAo
		suba_type���ӵ�ַ�ṹ	1�����ֽڵ�ַ	3��8+X�ṹ	2��˫�ֽڵ�ַ//2
		suba	       �������ڲ������ַ//
		*s	       ����Ҫд������ݵ�ָ��
		 num	       ����Ҫд������ݵĸ���
��  �� ֵ �� TRUE  - �����ɹ�
		 FALSE - ����ʧ��
**********************************************************************************************************/
uint8 I2C_WriteNByte(uint8 sla, uint8 suba_type, uint32 suba, uint8 s, uint32 num)
{
	if (num > 0)/* �����ȡ�ĸ���Ϊ0���򷵻ش��� */
	{	/* ���ò��� */	
		if (suba_type == 1)
		{	/* �ӵ�ַΪ���ֽ� */
			I2C_sla     	= sla;								/* �������Ĵӵ�ַ	 	*/
			I2C_suba    	= suba;								/* �����ӵ�ַ 			*/
			I2C_suba_num	= 1;								/* �����ӵ�ַΪ1�ֽ� 	*/
		}
		if (suba_type == 2)
		{	/* �ӵ�ַΪ2�ֽ� */
			I2C_sla     	= sla;								/* �������Ĵӵ�ַ 		*/
			I2C_suba   	 	= suba;								/* �����ӵ�ַ 			*/
			I2C_suba_num	= 2;								/* �����ӵ�ַΪ2�ֽ� 	*/
		}
		if (suba_type == 3)
		{	/* �ӵ�ַ�ṹΪ8+X */
			I2C_sla			= sla + ((suba >> 7 )& 0x0e);		/* �������Ĵӵ�ַ		*/
			I2C_suba		= suba & 0x0ff;						/* �����ӵ�ַ			*/
			I2C_suba_num	= 1;								/* �����ӵ�ַΪ8+X	 	*/
		}

	    I2C_buf     = s;										/* ���� 				*/
		I2C_num     = num;										/* ���ݸ��� 			*/
		I2C_suba_en = 2;										/* ���ӵ�ַ��д���� 	*/
		I2C_end     = 0;
		
		/* ���STA,SI,AA��־λ */
		I2CONCLR = 	(1 << 2)|	/* AA 	*/
					(1 << 3)|	/* SI 	*/
					(1 << 5);	/* STA 	*/
		
		/* ��λSTA,����I2C���� */
		I2CONSET = 	(1 << 5)|	/* STA 	*/
					(1 << 6);	/* I2CEN*/
		
		/* �ȴ�I2C������� */
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
�������� ��__irq IRQ_I2C()
�������� ��Ӳ��I2C�жϷ������ 
��ڲ��� ����
��  �� ֵ ����
��      ע ��ע�⴦���ӵ�ַΪ2�ֽڵ������ 
**********************************************************************************************************/
void __irq IRQ_I2C(void)
{	/* ��ȡI2C״̬�Ĵ���I2DAT */
	/* ����ȫ�ֱ��������ý��в��������������־ */
	/* ����ж��߼�,�жϷ��� */
	
	switch (I2STAT & 0xF8)
	{	/* ����״̬�������Ӧ�Ĵ��� */
		case 0x08:	/* �ѷ�����ʼ���� */				/* �����ͺ������ն��� 		*/
			/* װ��SLA+W����SLA+R */
		 	if(I2C_suba_en == 1)/* SLA+R */				/* ָ���ӵ�ַ�� 			*/
		 	{	I2DAT = I2C_sla & 0xFE; 				/* ��д���ַ 				*/
		 	}
            else	/* SLA+W */
            {  	I2DAT = I2C_sla;        				/* ����ֱ�ӷ��ʹӻ���ַ 	*/
            }
            /* ����SIλ */
            I2CONCLR =	(1 << 3)|						/* SI 						*/
            			(1 << 5);						/* STA 						*/
            break;
            
       	case 0x10:	/*�ѷ����ظ���ʼ���� */ 			/* �����ͺ������ն��� 		*/
       		/* װ��SLA+W����SLA+R */
       		I2DAT = I2C_sla;							/* �������ߺ��ط��ӵ�ַ 	*/
       		I2CONCLR = 0x28;							/* ����SI,STA */
       		break;

		case 0x18:
       	case 0x28:	/* �ѷ���I2DAT�е����ݣ��ѽ���ACK */
       		if (I2C_suba_en == 0)
       		{
	       		if (I2C_num > 0)
	       		{	I2DAT = I2C_buf;
	       		//I2DAT = *I2C_buf++;
	       			I2CONCLR = 0x28;					/* ����SI,STA 				*/
	       			I2C_num--;
	       		}
	       		else	/* û�����ݷ����� */
	       		{		/* ֹͣ���� */
	       		  	I2CONSET = (1 << 4);				/* STO 						*/
	       			I2CONCLR = 0x28;					/* ����SI,STA 				*/
	       		  	I2C_end = 1;						/* �����Ѿ�ֹͣ 			*/
	       		}
       		}
       		
            if(I2C_suba_en == 1)	/* ����ָ����ַ������������������ 				*/
            { 
            	if (I2C_suba_num == 2)
            	{	I2DAT = ((I2C_suba >> 8) & 0xff);
	       			I2CONCLR = 0x28;					/* ����SI,STA 				*/
	       			I2C_suba_num--;
	       			break;	
	       		} 
	       		
	       		if(I2C_suba_num == 1)
	       		{	I2DAT = (I2C_suba & 0xff);
	       			I2CONCLR = 0x28;					/* ����SI,STA 				*/
	       			I2C_suba_num--;
	       			break;	
	       		}
	       		
            	if (I2C_suba_num == 0)
            	{	I2CONSET = 0x20;
               		I2CONCLR = 0x08;
               		I2C_suba_en = 0;     				/* �ӵ�ַ������ 			*/
               		break;
               	}
            }
            
            if (I2C_suba_en == 2)/* ָ���ӵ�ַд,�ӵ�ַ��δָ��,�����ӵ�ַ 		*/
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
       		  
       case 0x40:	/* �ѷ���SLA+R,�ѽ���ACK */
       		if (I2C_num <= 1)	/* ��������һ���ֽ� */			
       		{	I2CONCLR = 1 << 2;      				/* �´η��ͷ�Ӧ���ź� 		*/
       		}
       		else
       		{ 	I2CONSET = 1 << 2;						/* �´η���Ӧ���ź� 		*/
       		}
       		I2CONCLR = 0x28;							/* ����SI,STA 				*/
       		break;

       	case 0x20:	/* �ѷ���SLA+W,�ѽ��շ�Ӧ��              */
       	case 0x30:	/* �ѷ���I2DAT�е����ݣ��ѽ��շ�Ӧ��     */
       	case 0x38:	/* ��SLA+R/W�������ֽ��ж�ʧ�ٲ�         */
   		case 0x48:	/* �ѷ���SLA+R,�ѽ��շ�Ӧ��              */
         	I2CONCLR = 0x28;
            I2C_end = 0xFF; 
       		break;   				
	
		case 0x50:	/* �ѽ��������ֽڣ��ѷ���ACK */
		       I2C_buf = I2DAT;
			//*I2C_buf++ = I2DAT;
			I2C_num--;
			if (I2C_num == 1)/* �������һ���ֽ� */
			{  	I2CONCLR = 0x2C;						/* STA,SI,AA = 0 			*/
			}
			else
			{ 
			 	I2CONSET = 0x04;						    /* AA=1 		*/
			  	I2CONCLR = 0x28;
			}
			break;
		
		case 0x58:	/* �ѽ��������ֽڣ��ѷ��ط�Ӧ�� */
		//	*I2C_buf++ = I2DAT;     					/* ��ȡ���һ�ֽ����� 		*/
		    I2C_buf = I2DAT; 
            I2CONSET = 0x10;        					/* �������� 				*/
            I2CONCLR = 0x28;
            I2C_end = 1; 
            break;
            
      	default:
      		break;
	}
   VICVectAddr = 0x00;              					/* �жϴ������ 			*/
}


