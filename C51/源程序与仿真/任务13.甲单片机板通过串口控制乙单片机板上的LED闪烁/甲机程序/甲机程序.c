/** 任务13.甲单片机板通过串口控制乙单片机板上的LED闪烁-甲机程序 **/
#include <stc.h>
#define uint unsigned int
#define uchar unsigned char
sbit LED1 = P0^0;
sbit LED2 = P0^3;
sbit K1 = P3^2;

void Delay(uint x)
{
 	uchar i;
	while(x--)
	{
	 	for(i=0;i<120;i++);
	}
}

void UartInit(void)		//9600bps@11.0592MHz
{
	PCON &= 0x7f;		//波特率不倍速
	SCON = 0x50;		//8位数据,可变波特率
	AUXR &= 0xbf;		//定时器1时钟为Fosc/12,即12T
	AUXR &= 0xfe;		//串口1选择定时器1为波特率发生器
	TMOD &= 0x0f;		//清除定时器1模式位
	TMOD |= 0x20;		//设定定时器1为8位自动重装方式
	TL1 = 0xFD;		//设定定时初值
	TH1 = 0xFD;		//设定定时器重装值
	ET1 = 0;		//禁止定时器1中断
	TR1 = 1;		//启动定时器1
}

void putc_to_SerialPort(uchar c)
{
 	SBUF = c;
	while(TI == 0);
	TI = 0;
}

void main()
{
 	uchar Operation_NO = 0;
	UartInit();			//串口初始化：9600bps@11.0592MHz
	while(1)
	{
	 	if(K1 == 0) 
		{
			Delay(8);
			if(K1 == 0)
			{
				while(K1==0);
				Operation_NO=(Operation_NO+1)%4;
			}
		}
		switch(Operation_NO)
		{
		 	case 0:
					LED1=LED2=1; break;
			case 1:
					putc_to_SerialPort('A');
					LED1=~LED1;LED2=1;break;
			case 2:
					putc_to_SerialPort('B');
					LED2=~LED2;LED1=1;break;
			case 3:
					putc_to_SerialPort('C');
					LED1=~LED1;LED2=LED1;break;
		}
		Delay(100);
	}
}