/******************** (C) COPYRIGHT 2012 Robottime ********************
* �ļ���    : amarino.c
* ����       : sinbad
* �汾       : V1.0.0
* ����       : 2012.3.28
* ������Ʒ : ̽����ARM7 LPC2138�����ư�
* ����       : ����׿�ֻ���amarino������͵��ֻ�����������
                  ����֧�֣�COMPASS��ORIENTATION
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
* ������    : Amarino_GetOrientation
* ����       : ��ȡOrientation��������ֵ
* ��ڲ��� : i�� 0 - Orientation ; 1 - pitch ; 2 - roll
* ����ֵ    : ��Ӧ��ֵ
*******************************************************************************/
int Amarino_GetOrientation(int i)
{
	return(orientation[i]);
}

/*******************************************************************************
* ������    : Filter_AntiPulse
* ����       : ��λֵƽ���˲��������������ƽ���˲�����
* ��ڲ��� : channel    �� �˲�ͨ��
                  new_data �� �½�����˲�����
* ����ֵ    : �˲����ֵ
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
* ������    : Amarino_Compass
* ����       : ȡָ���루��λ������
* ��ڲ��� : i �� buf�����ֽ���
                  j �� ���ݵ��ֳ�
		  k�� ѡ���˲�ͨ��
* ����ֵ    : ָ��������
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
* ������    : Amarino_Orientation
* ����       : ȡOrientation���������������ݣ���ֵ��orientation[3]
* ��ڲ��� : ��
* ����ֵ    : ��
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
* ������    : Amarino_Process
* ����       : ���ⲿ������ã�����amarino����
* ��ڲ��� : i      �� ���յ��ֽ�����
                  type �� ѡ���event������������ 1 - COMPASS �� 2 - ORIENTATION
* ����ֵ    : ��
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



