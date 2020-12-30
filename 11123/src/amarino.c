/******************** (C) COPYRIGHT 2012 Robottime ********************
* 文件名    : amarino.c
* 作者       : sinbad
* 版本       : V1.0.0
* 日期       : 2012.3.28
* 所属产品 : 探索者ARM7 LPC2138主控制板
* 功能       : 处理安卓手机上amarino软件发送的手机传感器数据
                  本版支持：COMPASS，ORIENTATION
********************************************************************/

int buf[40];     // ASCII Data received from the serial port
int buf_tmp = 0; // temp serial data
int buf_i   = 0; // Data received sequence

int orientation[3] = {0,0,0}; //save the orentation event. 0 - orentation; 1 - pitch ; 2 - roll

#define  N  12                      //filter effective
int filter_buf[3][N]; //filter buf, 3 stand for 3 channels of the filter

#define COMPASS       1
#define ORIENTATION 2

/*******************************************************************************
* 程序名    : Amarino_GetOrientation
* 功能       : 读取Orientation传感器的值
* 入口参数 : i： 0 - Orientation ; 1 - pitch ; 2 - roll
* 返回值    : 相应的值
*******************************************************************************/
int Amarino_GetOrientation(int i)
{
	return(orientation[i]);
}

/*******************************************************************************
* 程序名    : Filter_AntiPulse
* 功能       : 中位值平均滤波法（防脉冲干扰平均滤波法）
* 入口参数 : channel    ： 滤波通道
                  new_data ： 新进入的滤波数据
* 返回值    : 滤波后的值
*******************************************************************************/
int Filter_AntiPulse(int channel, int new_data)
{
  int i;
  int filter_max = 0; //max value
  int filter_min = 65535; //min value
  int filter_sum = 0; //sum of all value
  
  for(i=0;i<N-1;i++)  //update the filter
  {
    filter_buf[channel][i] = filter_buf[channel][i+1];
  }
  filter_buf[channel][N-1] = new_data;
  
  for(i=0;i<N;i++)    //get the max and min value
  {
    if( filter_buf[channel][i] > filter_max ) filter_max = filter_buf[channel][i];
    if( filter_buf[channel][i] < filter_min ) filter_min = filter_buf[channel][i];
  }
  
  for(i=0;i<N;i++)    //get sum of filter
  {
    filter_sum = filter_sum + filter_buf[channel][i];
  }
  
  return ( filter_sum - filter_max - filter_min ) / ( N - 2 ); //get rid of pulse(max and min), and get average.
}


/*******************************************************************************
* 程序名    : Amarino_Compass
* 功能       : 取指南针（方位）数据
* 入口参数 : i ： buf的总字节数
                  j ： 数据的字长
		  k： 选择滤波通道
* 返回值    : 指南针数据
*******************************************************************************/
int Amarino_Compass(int i,int j,int k) //i - total ASCII byte ; j - length of a value ; k - channel of filter
{
  int a = 0; 
  if ( j == 3 )  a = Filter_AntiPulse ( k, 100 * buf[i - j] + 10 * buf[i - j + 1] + buf[i - j + 2] ); //if value from 100 ~ 360
  if ( j == 2 )  a = Filter_AntiPulse ( k, 10 * buf[i - j] + buf[i - j + 1] );                        //if value from  10 ~ 99
  if ( j == 1 )  a = Filter_AntiPulse ( k, buf[i - j] );                                              //if value from   0 ~ 9
  //if ( j == 3 )  a = 100 * buf[i - j] + 10 * buf[i - j + 1] + buf[i - j + 2] ; //if value from 100 ~ 360
  //if ( j == 2 )  a = 10 * buf[i - j] + buf[i - j + 1] ;                        //if value from  10 ~ 99
  //if ( j == 1 )  a = k, buf[i - j] ;                                              //if value from   0 ~ 9
  return (a);
}


/*******************************************************************************
* 程序名    : Amarino_Orientation
* 功能       : 取Orientation传感器的三个数据，赋值给orientation[3]
* 入口参数 : 无
* 返回值    : 无
*******************************************************************************/
void Amarino_Orientation()
{
  int i = 0; 
  int j = 0;
  int value_pos[3][2];

  while( i < buf_i )
  {
	if( buf[i] == 17 || buf[i] == 11 ) value_pos[j][0] = i; //get 'A' or ';'
    if( buf[i] == -2 )  //get '.'
    {
      value_pos[j][1] = i;
	  
      if( buf[ value_pos[j][0] + 1 ] == -3 ) orientation[j] = -Amarino_Compass( value_pos[j][1] , value_pos[j][1] - value_pos[j][0] - 2 , j ); //if value is '-'
        else orientation[j] = Amarino_Compass( value_pos[j][1] , value_pos[j][1] - value_pos[j][0] - 1 , j ); //translate effective value to an int data   
	  j++;
    }
    if( j > 2 ) break;
	i++;
  }
}

/*******************************************************************************
* 程序名    : Amarino_Process
* 功能       : 供外部程序调用，处理amarino数据
* 入口参数 : i      ： 接收的字节数据
                  type ： 选择的event（传感器）。 1 - COMPASS ； 2 - ORIENTATION
* 返回值    : 无
*******************************************************************************/
void Amarino_Process(int i, int type)
{
    buf_tmp = i;       //get serial data
    if( buf_tmp != 19 )            //if one amarino Compass data isn't end
    {
      buf[buf_i++] = buf_tmp - 48; //save a ASCII byte to buf and translate to int.
    }
    else                           //if amarino sensor data send ending
    {
		if(type==COMPASS)
		{
			Perception_Amarino(Amarino_Compass(buf_i,buf_i-1,0),0,0);
		}
		if(type==ORIENTATION)
		{
			Amarino_Orientation();
			Perception_Amarino(orientation[0],orientation[1],orientation[2]);
		}
		buf_i = 0;                   //reset receive buf count
    }
}



