/*********************************************************
单片机控制4位数码管显示DS18B20的温度：
1.只用了3位数码管显示了个位、十位和百位，没有显示小数位。
2.数码管用P0控制，共阳极是低电平有效（即“0”亮），数码管的位由P2控制，也是低电平有效，
  分别用到了P2.1为百位，P2.2为十位，P2.3为个位
**********************************************************/
/** 任务15.用DS18B20测量温度并用数码管显示 **/
#include <AT89X51.H>

#define uint unsigned int
#define uchar unsigned char
sbit DQ = P3^5;
sbit P2_0 = P2^0;
sbit P2_1 = P2^1;
sbit P2_2 = P2^2;
sbit P2_3 = P2^3;
bit DS18B20_IS_OK = 1;
uchar CurrentT = 0;
uchar Temp_Value[]={0};
uchar Display_Digit[]={0,0,0,0};
uchar code DSY_CODE[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90};//数码管的段码,对应数字0~9

void delayms(uint x)	//ms延时子程序,延时x毫秒
{
 	uchar i;
	while(x--)
	{
	 	for(i=0;i<120;i++);
	}
}

void delayus(uchar us)	//us延时子程序，延时时间为“5+2*us”微妙
{
 	while(--us);
}

uchar Init_DS18B20()	//DS18B20初始化子程序
{
 	uchar status;
	DQ = 1;
	delayus(1);		//延时7us（实测）
	DQ = 0;
	delayus(250);	//延时505us（实测）
	DQ = 1;
	delayus(28);	//延时61us（实测）
	status = DQ;
	delayus(240);	//延时485us（实测）
	return status;	//根据返回值可以判断是否复位成功，“0”表示复位成功
}

uchar ReadOneByte()	//DS18B20读一个字节子程序
{
 	uchar i,dat=0;
	DQ = 1;
	for(i=0;i<8;i++)
	{
		DQ = 0;
		delayus(3);	//延时11us（实测）
		DQ = 1;
		dat >>= 1;
		if(DQ)
			dat|=0x80;
		DQ = 1;
		delayus(30);
	}
	return dat;
}

void WriteOneByte(uchar dat)	//DS18B20写入一个字节子程序
{
 	uchar i;
	for(i=0;i<8;i++)
	{
	 	DQ = 1;
		delayus(1);
		DQ = 0;
		delayus(1);
		DQ = (bit)(dat&0x01);
		dat >>= 1;
		delayus(40);		
	}
}

void Display_Temperature()	//数码管显示温度子程序
{
 	uchar i;
	for(i=0;i<140;i++)	//输出到数码管显示
	{
		P0 = 0xff;	//关闭数码管，防止闪烁
		P2_1 = 0;	//低电平对应的位显示
		P0 = DSY_CODE[Display_Digit[2]];
		delayms(2);

		P0 = 0xff;	//关闭数码管，防止闪烁
		P2_1 = 1;P2_2 = 0;	//低电平对应的位显示
		P0 = DSY_CODE[Display_Digit[1]];
		delayms(2);

		P0 = 0xff;	//关闭数码管，防止闪烁
		P2_2 = 1;P2_3 = 0;	//低电平对应的位显示
		P0 = DSY_CODE[Display_Digit[0]];
		delayms(2);
		P2_3 = 1;
	}
}

void Read_Temperature()
{
 	if(Init_DS18B20()==1)	//判断是否复位成功
		DS18B20_IS_OK=0;	//DS18B20没有准备好
	else
	{
		WriteOneByte(0xcc);
		WriteOneByte(0x44);
		DQ = 1;
		Display_Temperature();	//等待温度转换,耗时约840ms,边等边显示上次测量到的温度
		Init_DS18B20();
		WriteOneByte(0xcc);
		WriteOneByte(0xbe);
		Temp_Value[0] = ReadOneByte();	//读取RAM的第0字节
		Temp_Value[1] = ReadOneByte();	//读取RAM的第1字节
		CurrentT = ((Temp_Value[0]&0xf0)>>4)|((Temp_Value[1]&0x07)<<4);
		Display_Digit[2] = CurrentT/100;
		Display_Digit[1] = CurrentT%100/10;
		Display_Digit[0] = CurrentT%10;
		DS18B20_IS_OK=1;	
	}	
}

void main()	//主程序
{
	while(1)
	{
		Read_Temperature();	//实时读取温度并显示
	}
}