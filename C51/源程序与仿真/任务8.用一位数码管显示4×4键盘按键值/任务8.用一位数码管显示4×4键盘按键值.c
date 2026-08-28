/** 任务8.用一位数码管显示4×4键盘按键值 **/
//==声明区=====================================
#include <reg52.h>							//定义头文件
#define KEYP P1
#define SEG7P P0
char code TAB[16]={	0xc0,0xf9,0xa4,0xb0,	//“0-3”对应的段码
					0x99,0x92,0x82,0xf8,	//“4-7”对应的段码
					0x80,0x90,0xa0,0x83,	//“8-b”对应的段码
					0xa7,0xa1,0x84,0x8e};	//“c-f”对应的段码
unsigned char disp =0x7f;					//声明显示初值为小数点“.”
unsigned char scan[4]={0xef,0xdf,0xbf,0x7f};//高4位为扫描码，低4位设置为输入
void delay1ms(int);

//==主程序区====================================
void main()
{
	unsigned char row,col;					//row:行，col：列
	unsigned char colkey,kcode;				//colkey:列键值，kcode：按键码
	P2=0xf7;								//P2.3为0，让最右边数码管显示
	while(1)
	{
		for(row=0;row<4;row++)				//第row次循环，扫描第row行
		{
			KEYP=scan[row];					//高4位输出扫描信号，低4位输入行值
			SEG7P=disp;						//把disp储存的数字输出
			colkey=~KEYP&0x0f;				//读入KEYP低4位(反相后清除高4位)
			if(colkey!=0)					//若有按键按下
			{
				if(colkey==0x01) col=0;		//若第0列被按下
				else if(colkey==0x02) col=1;//若第1列被按下
				else if(colkey==0x04) col=2;//若第2列被按下
				else if(colkey==0x08) col=3;//若第3列被按下
				kcode =4*row+col;			//算出按键号码
				disp=TAB[kcode];			//把将要显示的值存入disp
				while(colkey!=0)			//当按钮未松开一直等
				{	colkey=~KEYP&0x0f;}
			}
			delay1ms(1);
		}	
	}
}

//==定义子程序区================================
void delay1ms(int x)						//延时1ms的子程序
{
 	unsigned char i,j;
	for(i=0;i<x;i++)
		for(j=0;j<120;j++);
}