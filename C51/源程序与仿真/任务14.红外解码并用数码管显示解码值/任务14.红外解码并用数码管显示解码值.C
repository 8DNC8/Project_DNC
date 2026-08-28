/*程序说明： 
51单片机红外遥控解码程序
单片机采用外部中断INT0管脚和红外接收头的信号线相连，
中断方式为边沿触发方式。并用定时器0计算中断的间隔时间，
来区分前导码、二进制的1,0;8位操作码提取出来在数码管上显示。
解码值在Im[2]中，当IrOK=1时解码有效。 
用遥控器对准红外接收头，按下遥控器按键，在数码管的两位上就会显示对应按键的编码
*/
/** 任务14.红外解码并用数码管显示解码值-红外接收部分 **/
#include <stc.h>
#define uchar unsigned char
#define uint unsigned int
#define ms15 15000	//15ms 此处为晶振为12M时的取值, 如用其它频率的晶振时,要改变相应的取值。
#define ms7 7000	//7ms
#define ms1_5 1500	//1.5ms
#define ms_7 700 	//0.7ms
#define ms3 3000	//3ms
sbit P2_2 = P2^2;
sbit P2_3 = P2^3;
unsigned char code TAB[16]={0xc0,0xf9,0xa4,0xb0,	//“0-3”对应的段码
							0x99,0x92,0x82,0xf8,	//“4-7”对应的段码
							0x80,0x90,0xa0,0x83,	//“8-b”对应的段码
							0xa7,0xa1,0x84,0x8e};	//“c-f”对应的段码
uchar f;
uchar Im[4]={0x00,0x00,0x00,0x00};
uchar show[2]={0x00,0x00};
uint Tc;
uchar m,IrOK;
void delay(unsigned int T)
{
	unsigned int CON;
	unsigned int i;
	for(i=0;i<T;i++)
		for(CON=0;CON<120;CON++);
}
void display()
{
   P0=0xff;
   P2_2 = 0;P2_3 = 1;
   P0=TAB[show[0]];
   delay(1);
   P0=0xff;
   P2_2 = 1;P2_3 = 0;
   P0=TAB[show[1]];
   delay(1);
}


void intersvr0(void) interrupt 0	//外部中断解码程序
{
	Tc=TH0*256+TL0;	//提取中断时间间隔时长
	TH0=0; 
	TL0=0;	//定时中断重新置零
	if((Tc>ms7)&&(Tc<ms15))	//找到启始码
	{ 
		m=0;
		f=1;
		return;
	}
	if(f==1)
	{
		if(Tc>ms1_5&&Tc<ms3) 
		{
			Im[m/8]=Im[m/8]>>1|0x80; m++; 
		}
		if(Tc>ms_7&&Tc<ms1_5) 
		{
			Im[m/8]=Im[m/8]>>1; m++; //取码
		}
		if(m==32) 
		{
			m=0;  
			f=0;
			if(Im[2]==~Im[3]) 
			{
				IrOK=1; 
			}
			else IrOK=0;	//取码完成后判断读码是否正确
		}//准备读下一码	
	}
}

void main(void)
{
	m=0;
	f=0;
	EA=1;
	IT0=1;
	EX0=1; 
	TMOD=0x11;  
	TH0=0;TL0=0;
	TR0=1;
	while(1)
	{
		if(IrOK==1) 
		{
			show[1]=Im[2] & 0x0F;     //取键码的低四位
			show[0]=Im[2] >> 4;  
			IrOK=0;	 
		}
		display();
	}
}

