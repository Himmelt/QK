#include <STC15F2K60S2.H>
#include <intrins.h>
#include "music.h"
#include "texts.h"
/*位变量声明*/
sbit spk = P3 ^ 7;		   //音频输出端
sbit button = P3 ^ 3;	  //INT1,中断输入端
bit disflag = 0;		   //显示内容块标志位
unsigned char songnum = 0; //音乐序号标志位
/*全局指针数组*/
code unsigned char *text[2] = {texts, icou};
code unsigned char *song[2] = {birthday, tkzc};
/*全局变量*/
unsigned char m = 0, qk = 0;
/*button INT1,中断长按信号识别*/
void diswitch() //@12.000MHz
{
	unsigned char i, j, k;
	_nop_();
	i = 40;
	j = 250;
	k = 187;
	do
	{
		do
		{
			while (--k)
				;
		} while (--j);
		if (button == 1)
		{
			TR1 = 1;
			return;
		}
	} while (--i);
}
/*一拍延时&内容块切换判断*/
void onejp()
{
	unsigned int i, j;
	for (i = 0; i < 200; i++)
		for (j = 0; j < 250; j++)
			;
	if (button == 0)
		qk++;
	if (qk >= 20)
	{
		qk = 0;
		TR1 = 0;
		disflag = disflag;
		diswitch();
		TR1 = 1;
	}
}

/*主函数*/
void main()
{
	unsigned char i = 0, j = 0, jp = 0, fr = 0, num = 0;
	//各中断源初始化
	EA = 1;
	EX0 = 1;
	EX1 = 1;
	ET0 = 1;
	ET1 = 1;
	IT0 = 1;
	IT1 = 1;
	//10ms@12.000MHz
	AUXR &= 0x7F;
	AUXR &= 0xBF; //定时器时钟12T模式
	TMOD = 0x00;
	TL0 = 0xF0;
	TH0 = 0xD8;
	TF0 = 0;
	TF1 = 0;
	//主循环
	while (1)
	{
		num = songnum;
		i = 0;
		while ((song[num][i] | 0x0f) != 0xff)
		{
			fr = (song[num][i] >> 4) - 1;
			TH1 = F_H[fr];
			TL1 = F_L[fr];
			TR1 = 1;
			if (TR0 == 0)
			{
				if (fr < 4)
				{
					P2 = 0xff << (2 * fr);
					P1 = 0xff;
					P0 = 0xff;
				}
				else if (fr < 8)
				{
					P2 = 0x00;
					P1 = 0xff << (2 * (fr - 4));
					P0 = 0xff;
				}
				else
				{
					P2 = 0x00;
					P1 = 0x00;
					P0 = 0xff << (2 * (fr - 8));
				}
			}
			jp = (song[num][i] & 0x0f) * 4;
			for (j = 0; j < jp; j++)
				onejp();
			i++;
			TR1 = 0;
			if (songnum != num)
				break;
		}
		spk = 0;
	}
}
//INT1,P3.3,button,过程切换,最高中断//必须下降沿触发
void song_switch() interrupt 2
{
	songnum++;
	if (songnum >= 2)
		songnum = 0; //切换音乐标志位
}

//INT0,P3.2,左开关,任务:启动T0//跳沿触发一次
void left() interrupt 0
{
	TR0 = 1;
}

//T0,任务切换,16位自载入
void task_switch() interrupt 1
{
	if (text[disflag][3 * m] == 0xaa)
	{
		TR0 = 0;
		m++;
	}
	else if (text[disflag][3 * m] == 0x55)
	{
		TR0 = 0;
		m = 0;
	}
	else
	{
		P0 = text[disflag][3 * m];
		P1 = text[disflag][3 * m + 1];
		P2 = text[disflag][3 * m + 2];
		m++;
	}
}
//T1,音符发生器,16位自载入
void tune() interrupt 3 using 1
{
	spk = !spk;
}
