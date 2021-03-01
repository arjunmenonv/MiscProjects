#include<lpc23xx.h>
#include<stdio.h>
#include<stdlib.h>

//Variable Declarations
long int thr = 8999999;
int inp1,inp2,pore;
int eps[2],E,Y;
int k_p=1,k_d=0,k_i=0;
int out[70]; //P[70]
int flag,i=0,j=0;
long int corr;	//Correlation Value at n = 0
int sc_fac, sig_scale; 	//Scaling Factor and Variance for generating noisy patient signal 
int k; //Running variable for for loop 
int inter_ps;
int count;
// Look Up Table Definitions
int isig[70] = {160, 200, 240, 280, 320, 360, 400, 440, 480, 520, 560, 600, 640, 680, 720, 680, 640, 600, 560, 520, 480, 440, 400, 360,320, 280, 240, 200, 160, 120, 150, 180, 210, 240, 270, 300, 330, 360, 390, 420, 450, 480, 510, 540, 570, 600, 630, 660,690, 720, 690, 660, 630, 600, 570, 540, 510, 480, 450, 420, 390, 360, 330, 300, 270, 240, 210, 180, 150, 120};
int psig[70];
int noise[150] = {8, 4, 1, -2, 1, -2, 6, 4, 3, -5, 1, 1, 9, 0, 0, 1, -5, 1, -4, -2, -2, 2, -2, 5, 1, 2, 1, -4, -8, -2, 6, 3, 0, 0, -1,0, 0, 2, 0, 2, 2, -3, -5, 4, -5, -8, -12, -3, 5, -6, -1, -3, 0, -1, -2, 5, 1, -4, -4, 0, 3, 0, 1, 5, -3, 3, 3, -1, 2,3, 2, -4, 4, 0, 1, -4, 1, 2, 0, -6, -3, -5, 0, 1, 0, 0, -2, -3, -6, -5, 0, -3, 2, 6, -2, -3, -1, 1, 7, -8, 2, -6, 4,0, -1, 1, 4, 4, -3, -6, 0, -3, 0, 0, -5, -3, 10, 5, 0, -2, 1, 0, 4, -4, 3, 3, -4, -3, 7, 0, 0, 4, 0, -8, 0, 6, 8, 1,-7, -2, -5, 1, 6, 1, 1, -3, 9, 2, -1, 5};											
// Function Definitions
int find_corr()
{
	corr = 0;
	k = 0;
	for(;k<70; k++)
	{
		corr += isig[k]*psig[k];
	}
	return corr;
}
void rsignal(int sc_fac, int sig_scale)
	{
		int index = 45396%150;
		k = 0;
		for(; k<70; k++)
		{
			if(index+k == 150)
			{
				index -= 150;
			}
		inter_ps = sc_fac*isig[k];
		
		for(count = 0;inter_ps>=0;count++)
			inter_ps = inter_ps-10;
		psig[k] =  count + noise[index+k]*sig_scale;
		}
	}
	
void id_switch(int inp1)
{
	int dummy1, dummy2,dummy3;
	dummy1 = inp1&0x03;
	dummy2 = (inp1>>2)&0x03;
	dummy3 = (inp1>>4)&0x03;
	switch(dummy1)
	{
		case 0: sc_fac = 9;
						break;
		case 1: sc_fac = 7;
						break;
		case 2: sc_fac = 5;
						break;
		case 3: sc_fac = 3;
						break;
		default: break;
	}
	switch(dummy2)
	{
		case 0: sig_scale = 0;
						break;
		case 1: sig_scale = 1;
						break;
		case 2: sig_scale = 2;
						break;
		case 3: sig_scale = 3;
						break;
		default: break;
	}
	switch(dummy3)
	{
		case 0: flag=1;
						break;
		case 1: flag=2;
						break;
		case 2: flag=3;
						break;
		case 3: flag=4;
						break;
	}
}
void BPPfunc()
{
	out[0]=psig[0];
	eps[0]=0;
	eps[1]=isig[0]-psig[0];
	E=eps[1];
	Y=(k_p+k_d+k_i)*eps[1];
	for(i=1;i<70;i++)
	{
		out[i]=out[i-1]+Y+psig[i]-psig[i-1];
		eps[0]=eps[1];
		eps[1]=isig[i]-out[i];
		Y=k_p*eps[1];
		Y+=k_d*(eps[1]-eps[0]);
		Y+=k_i*(E+eps[1]);
		E=E+eps[1];
	}
}
void delay(int k)
{
	int i,j;
	for(i=0;i<k;i++)
	for(j=0;j<0x0F;j++);
}
void stmot()
{
	IODIR0=0xFFFFFFFF;
	for(k=0;k<20;k++)
		{
			IOPIN0=0x280;
			delay(0x90);
			IOPIN0=0x180;
			delay(0x90);
			IOPIN0=0x140;
			delay(0x90);
			IOPIN0=0x240;
			delay(0x90);
		}
}

int main()
{
	while(1)
	{
		FIO3DIR=0xFF;
		FIO4DIR=0x00;
		inp1=FIO4PIN;
		pore=inp1&0x00000001;
		inp1=inp1>>1; 	//Contains switch input as (b2, b1) -> sc_factor; (b3, b4) -> sig_factor 
		id_switch(inp1);
		rsignal(sc_fac, sig_scale);
		corr=find_corr();
		if(corr>=thr)
		{
			FIO3PIN=0x01;
			switch(flag)
			{
				case 1: while(pore)
								{
									PINSEL1 = 0x00200000;
									i=0;
									while(1)
									{
										DACR=psig[i]<<6;
										if(i==69)
												i=0;
										else i++;
										delay(1);
									}
								}
								break;
				case 2:	while(pore)
								{
									PINSEL1 = 0x00200000;
									i=0;
									while(1)
									{
										DACR=isig[i]<<6;
										if(i==69)
												i=0;
										else i++;
										delay(1);
									}
								}
								break;
				default:break;
				}
		}
		else 
		{
			FIO3PIN=0x02;
			switch(flag)
			{
				case 1: while(pore)
								{
									BPPfunc();
									PINSEL1 = 0x00200000;
									i=0;
									while(1)
									{
										DACR=out[i]<<6;
										if(i==69)
												i=0;
										else i++;
										delay(1);
									}
								}
								break;
				case 2:	while(pore)
								{
									PINSEL1 = 0x00200000;
									i=0;
									while(1)
									{
										DACR=isig[i]<<6;
										if(i==69)
												i=0;
										else i++;
										delay(1);
									}
								}
							break;
				case 3: while(pore)
								{
									PINSEL1 = 0x00200000;
									i=0;
									while(1)
									{
										DACR=psig[i]<<6;
										if(i==69)
												i=0;
										else i++;
										delay(1);
									}
								}
								break;
				case 4: while(pore)
								{
									stmot();
								}
								break;
				default:break;
			}
		}
	}
	return 1;
}
