#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <poll.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>

#define INPUT 0	
#define OUTPUT 1	
#define LOW 0	
#define HIGH 1	
#define BLOCK_SIZE (4*1024)	
#define BCM2708_PERI_BASE 0x3F000000
#define GPIO_BASE (BCM2708_PERI_BASE + 0x00200000)
#define AUX_BASE (BCM2708_PERI_BASE + 0x00215000)

#define GPPUD           (0x00200094)
#define GPPUDCLK0       (0x00200098)

#define AUX_ENABLES     (0x00215004)
#define AUX_MU_IO_REG   (0x00215040)
#define AUX_MU_IER_REG  (0x00215044)
#define AUX_MU_IIR_REG  (0x00215048)
#define AUX_MU_LCR_REG  (0x0021504C)
#define AUX_MU_MCR_REG  (0x00215050)
#define AUX_MU_LSR_REG  (0x00215054)
#define AUX_MU_MSR_REG  (0x00215058)
#define AUX_MU_CNTL_REG (0x00215060)
#define AUX_MU_BAUD_REG (0x00215068)

static volatile unsigned int *gpio;
static volatile unsigned int *aux;

void pin_Mode(int mode);
int digitalRead_pin();
void init();

int addrToOffset(int addr)
{
	int res;
	addr = addr & 0xFFF;
	res = addr / 4;
	return res;
}

int main(){
	int fd;
	if((fd = open("/dev/mem",O_RDWR |  O_SYNC))<0){
		printf("Fail open /dev/mem\n");
		}
	gpio = (unsigned int*)mmap(0,BLOCK_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,fd,GPIO_BASE);
	aux = (unsigned int*)mmap(0,BLOCK_SIZE, PROT_READ|PROT_WRITE,MAP_SHARED,fd,AUX_BASE);

	//printf("%x  ",(aux));
	init();
	pin_Mode(INPUT);
	/*
	 while(1){
	printf("%d\n",digitalRead_pin_15());
	sleep(1);
	}*/
	return 0;
}

void init()
{
	int sel = addrToOffset(AUX_ENABLES);
	*(aux+sel) = 0x1;//AUX_ENABLES
	sel =  addrToOffset(AUX_MU_IER_REG);
	*(aux+sel) = 0x0;//AUX_MU_IER_REG
	sel =  addrToOffset(AUX_MU_CNTL_REG);
	*(aux+sel) = 0x0;//AUX_MU_CNTL_REG
	sel =  addrToOffset(AUX_MU_LCR_REG);
	*(aux+sel) = 0x3;//AUX_MU_LCR_REG
	sel =  addrToOffset(AUX_MU_MCR_REG);
	*(aux+sel) = 0x0;//AUX_MU_MCR_REG
	sel =  addrToOffset(AUX_MU_IIR_REG);
	*(aux+sel) = 0xC6;//AUX_MU_IIR_REG
	sel =  addrToOffset(AUX_MU_BAUD_REG);
	*(aux+sel) = 0x10E;//AUX_MU_BAUD_REG
	printf("%d\n",*(aux+sel));
}

void pin_Mode(int mode)
{
	int fSel;
	fSel= 1;
	unsigned int TXport = 14;
	unsigned int TXshift = 12;//pin14
	//unsigned int RXshift = 15;//pin15
	int gSel;
	if(mode==INPUT)
	{
		*(gpio+fSel) = (*(gpio+fSel) & ~(7<<TXshift));
		*(gpio+fSel) = (*(gpio+fSel) | (2<<TXshift));
	/*	*(gpio+fSel) = (*(gpio+fSel) & ~(7<<RXshift));
		*(gpio+fSel) = (*(gpio+fSel) | (2<<RXshift));*/

		gSel = addrToOffset(GPPUD);
		*(gpio+gSel) = (*(gpio+gSel)& 0);//GPPUD
		gSel = addrToOffset(GPPUDCLK0);
		sleep(1);
		//*(gpio+gSel) = (*(gpio+gSel) & (1<<14)|(1<<15));//GPPUDCLK0
		*(gpio+gSel) = (1<<TXport);//GPPUDCLK0
	printf("GPPUDCLK0: %d\n",*(gpio + gSel));
		sleep(1);
		*(gpio+gSel) = (*(gpio+gSel) & 0);//GPPUDCLK0

		gSel = addrToOffset(AUX_MU_CNTL_REG);
		*(aux+gSel) =  3;
	printf("cntl_reg: %d\n",*(aux + gSel));

		char data = 'a';
		gSel = addrToOffset(AUX_MU_IO_REG);
		while(1)
		{
			*(aux+gSel) = data;
			printf("buff: %c\n",*(aux+gSel));
			//while(1)
			//	if(digitalRead_pin()&0x20) break;
			sleep(1);	
			printf("%c\n",data);
		}
	}
/*	else if(mode==OUTPUT)
		*(gpio+fSel) = (*(gpio+fSel)& ~(7<<shift))|(1<<shift);*/
}

int digitalRead_pin()
{
	int level = addrToOffset(AUX_MU_LSR_REG);
	int value;
	if((value=(*(aux + level)))!=0)
	{
		printf("HIGH\n");
		return value;
	}
	else 
	{
		return 0;
	}
}
