/** 任务14.红外解码并用数码管显示解码值-模拟红外发射部分 **/
#include <AT89X51.h> 
#define KEYP P1
#define SEG7P P0
static unsigned int count;      //延时计数器
static unsigned int endcount; 	//终止延时计数
char iraddr1;  //十六位地址的第一个字节
char iraddr2;  //十六位地址的第二个字节
void SendIRdata(char p_irdata);
void getkey();

void main(void) 
{
  EA = 1; //允许CPU中断 
  TMOD = 0x11; //设定时器0和1为16位模式1 
  ET0 = 1; //定时器0中断允许 
  TH0 = 0xFF; 
  TL0 = 0xE6; //设定时值0为38K 也就是每隔26us中断一次  
  TR0 = 1;//开始计数
  iraddr1=0xff;
  iraddr2=0xff; 
  while(1)
  {
    getkey();
  }
} 
//定时器0中断处理 
void timeint(void) interrupt 1 
{ 
  TH0=0xFF; 
  TL0=0xE6; //设定时值为38K 也就是每隔26us中断一次
  count++;
} 
void SendIRdata(char p_irdata)
{
	int i;
	char irdata;	
	endcount=223;//发送9ms的起始码
	count=0;
	P3_4=1;
	while(count<endcount);	
	endcount=117;//发送4.5ms的结果码
	count=0;
	P3_4=0;
	while(count<endcount);	
	irdata=iraddr1;
	for(i=0;i<8;i++)//发送十六位地址的前八位
	{
		//先发送0.56ms的38KHZ红外波（即编码中0.56ms的低电平）
		endcount=13;
		count=0;
		P3_4=1;
		while(count<endcount);
		//停止发送红外信号（即编码中的高电平）
		if(irdata%2)  //判断二进制数个位为1还是0
		{
			endcount=39;  //1为宽的高电平
		}
		else
		{
			endcount=13;   //0为窄的高电平
		}
		count=0;
		P3_4=0;
		while(count<endcount);
		irdata=irdata>>1;
	}
	irdata=iraddr2;
	for(i=0;i<8;i++)//发送十六位地址的后八位
	{
		//先发送0.56ms的38KHZ红外波（即编码中0.56ms的低电平）
		endcount=13;
		count=0;
		P3_4=1;
		while(count<endcount);
		//停止发送红外信号（即编码中的高电平）
		if(irdata%2)  //判断二进制数个位为1还是0
		{
			endcount=39;  //1为宽的高电平
		}
		else
		{
			endcount=13;   //0为窄的高电平
		}
		count=0;
		P3_4=0;
		while(count<endcount);
		irdata=irdata>>1;
	}	
	irdata=p_irdata;
	for(i=0;i<8;i++)//发送八位数据
	{
		//先发送0.56ms的38KHZ红外波（即编码中0.56ms的低电平）
		endcount=13;
		count=0;
		P3_4=1;
		while(count<endcount);
		//停止发送红外信号（即编码中的高电平）
		if(irdata%2)  //判断二进制数个位为1还是0
		{
			endcount=39;  //1为宽的高电平
		}
		else
		{
			endcount=13;   //0为窄的高电平
		}
		count=0;
		P3_4=0;
		while(count<endcount);

		irdata=irdata>>1;
	}
	irdata=~p_irdata;
	for(i=0;i<8;i++)//发送八位数据的反码
	{
		//先发送0.56ms的38KHZ红外波（即编码中0.56ms的低电平）
		endcount=13;
		count=0;
		P3_4=1;
		while(count<endcount);
		//停止发送红外信号（即编码中的高电平）
		if(irdata%2)  //判断二进制数个位为1还是0
		{
			endcount=39;  //1为宽的高电平
		}
		else
		{
			endcount=13;   //0为窄的高电平
		}
		count=0;
		P3_4=0;
		while(count<endcount);
		irdata=irdata>>1;
	}
	endcount=50;
	count=0;
	P3_4=1;
	while(count<endcount);
	P3_4=0;
}
void getkey()
{
	unsigned char row,col;				//row:行，col：列
	unsigned char colkey,kcode;			//colkey:列键值,kcode：按键码
	unsigned char scan[4]={0xef,0xdf,0xbf,0x7f};//高4位为扫描码，低4位设置为输入
	for(row=0;row<4;row++)				//第row次循环，扫描第row行
	{
		KEYP=scan[row];					//高4位输出扫描信号，低4位输入行值
		colkey=~KEYP&0x0f;				//读入KEYP低4位(反相后清除高4位)
		if(colkey!=0)					//若有按键按下
		{
			if(colkey==0x01) col=0;		//若第0列被按下
			else if(colkey==0x02) col=1;//若第1列被按下
			else if(colkey==0x04) col=2;//若第2列被按下
			else if(colkey==0x08) col=3;//若第3列被按下
			kcode =4*row+col;			//算出按键号码
			while(colkey!=0)			//当按钮未松开一直等
			{	colkey=~KEYP&0x0f;}
			SendIRdata(kcode);
		}
	}	
}