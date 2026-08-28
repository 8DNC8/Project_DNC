/** 任务12.通过串口发送一串字符至电脑 **/
#include "reg52.h"
#include "intrins.h"
#define uchar unsigned char
#define uint  unsigned int
 
void Com_Init(void)		//串口初始化子程序
{
	PCON &= 0x7f;		//波特率不倍速
	SCON = 0x50;		//8位数据,可变波特率
	TMOD &= 0x0f;		//清除定时器1模式位
	TMOD |= 0x20;		//设定定时器1为8位自动重装方式
	TL1 = 0xFD;			//设定定时初值
	TH1 = 0xFD;			//设定定时器重装值
	ET1 = 0;			//禁止定时器1中断
	TR1 = 1;			//启动定时器1		
}

void Main()
{
	uchar i = 0;
	uchar code Buffer[] = "Welcome to study 51.\r\n";	 //所要发送的数据
	uchar *p;
	Com_Init();
	p = Buffer;
	while(1)
	{		
		SBUF = *p;
		while(!TI)                   //如果发送完毕，硬件会置位TI
		{
			_nop_();	
		}
		p++;
		if(*p == '\0') break;		//在每个字符串的最后，会有一个'\0'
		TI = 0;		                //TI清零
	}
	while(1);
}