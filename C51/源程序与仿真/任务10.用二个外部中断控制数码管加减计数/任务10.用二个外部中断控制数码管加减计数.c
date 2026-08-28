/**任务10.用二个外部中断控制数码管加减计数**/
#include <reg52.h>			//	定义头文件
#define	SEG7 P0				//	定义7段数码管接至P0
char code TAB[10]={	0xc0, 0xf9, 0xa4, 0xb0, 0x99,	// 数字0-4
					0x92, 0x83, 0xf8, 0x80, 0x98};	// 数字5-9
void delay1ms(int);			//	声明延迟函数 

main()						//	主程序开始 
{		
	P2=0xf7;				//	P2.3为0，让最右边数码管显示
	IE=0x85;				//	打开外部中断INT0和INT1
	IP=0x04;				//	设置INT1优先级高于INT0
	SEG7=0xbf;				//	数码管初始时显示“-
	while(1);				//	无穷等待,主程序无任何动作
}							//	主程序结束 
// INT 0的中断子程序 - 数码管从0加到9
void add_int0(void) interrupt 0//INT0中断子程序开始 
{	char i;
	unsigned saveSEG7=SEG7;	//	储存中断前数码管状态 
	for(i=0;i<10;i++)		//	显示0-9,共10次循环 
		{	
			SEG7=TAB[i];	//	显示数字,使用实验板需改为"SEG7=~TAB[i];" 
			delay1ms(500);	//	延迟500ms 
		}					//	for循环结束;
	SEG7=saveSEG7;			//	写回中断前数码管状态 
}							//	结束INT0 中断子程序 
// INT 1的中断子程序 - 数码管从9减到0
void subb_int1(void) interrupt 2//INT0中断子程序开始 
{	char i;
	unsigned saveSEG7=SEG7;	//	储存中断前数码管状态 
	for(i=9;i>=0;i--)		//	显示0-9,共10次循环 
		{	
			SEG7=TAB[i];	//	显示数字,使用实验板需改为"SEG7=~TAB[i];" 
			delay1ms(500);	//	延迟500ms 
		}					//	for循环结束;
	SEG7=saveSEG7;			//	写回中断前数码管状态 
}							//	结束INT0 中断子程序 
// 延迟函数,延迟约x ms
void delay1ms(int x)		//	延迟函数开始 
{	int i,j;				//	声明整数变数i,j 
	for (i=0;i<x;i++)		//	计数x次,延迟x ms 
		for (j=0;j<120;j++);//	计数120次，延迟1ms 
}							//	延迟函数结束 