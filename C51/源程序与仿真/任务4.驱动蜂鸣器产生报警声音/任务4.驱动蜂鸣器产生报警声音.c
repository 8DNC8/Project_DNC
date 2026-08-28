/** 任务4.驱动蜂鸣器产生报警声音  **/
//==声明区====================================
#include	<reg51.h>		//  定义头文件
sbit buzzer = P3^6 ;		//  声明蜂鸣器的位置为P3.6
void delay(int);			//	声明延迟函数 
void pulse_BZ(int,int,int);	//	声明蜂鸣器发声函数 
//==主程序====================================
main()						//	主程序开始 
{	while(1)				//	无穷循环,程序一直跑 
	{	pulse_BZ(100,1,1);	//  蜂鸣器发声1kHz声音100ms
		pulse_BZ(100,2,2);	//  蜂鸣器发声500Hz声音200ms
	}						//	while循环结束 
}							//	主程序结束 
//==子程序=====================================
/* 延迟函数开始,延迟x 0.5ms */ 
void delay(int x)			//	延迟函数开始 
{	int i,j;				//	声明整数变量i,j
	for (i=0;i<x;i++)		//	计数x次,延迟约x 0.5ms 
		for (j=0;j<60;j++);	//	计数60次，延迟约0.5ms 
}							//	延迟函数结束 
/* 蜂鸣器发声函数,count=计数次数,TH=高态时间,TL=低态时间 */
void pulse_BZ(int count,int TH,int TL)	//	蜂鸣器发声函数开始 
{	int i;					//	声明整数变数i
	for(i=0;i<count;i++)	//	计数count次 
	{	buzzer=1;			//	输出高电平
		delay(TH);			//	延迟TH 0.5ms 
		buzzer=0;			//	输出低电平 
		delay(TL);			//	延迟TL 0.5ms 
	}						//	for循环结束 
}							//	蜂鸣器发声函数结束 