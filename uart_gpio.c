#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>

#define INPUT 0	
#define OUTPUT 1	
#define LOW 0	
#define HIGH 1	
#define BLOCK_SIZE (4*1024)	
#define BCM2708_PERI_BASE 0x3F000000
#define GPIO_BASE (BCM2708_PERI_BASE + 0x00200000)
#define AUX_BASE (BCM2708_PERI_BASE + 0x00215000)

#define AUX_ENABLES     (0x00215004)
#define AUX_MU_IO_REG   (0x00215040)
#define AUX_MU_IER_REG  (0x00215044)
#define AUX_MU_IIR_REG  (0x00215048)
#define AUX_MU_LCR_REG  (0x0021504C)
#define AUX_MU_MCR_REG  (0x00215050)
#define AUX_MU_CNTL_REG (0x00215060)
#define AUX_MU_BAUD_REG (0x00215068)

static volatile unsigned int *gpio;
static volatile unsigned int *aux;

int addrToOffset(int addr)
{
	int res;
	addr = addr & 0xFFF;
	res = addr / 4;
	return res;
}

void init(int baud)
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
	*(aux+sel) = baud;//AUX_MU_BAUD_REG
	printf("%d\n",*(aux+sel));
}

void pin_Mode()
{
	int fSel;
	fSel= 1;
	unsigned int TXport = 14;
	unsigned int TXshift = 12;//pin14

	int gSel;
	*(gpio+fSel) = (*(gpio+fSel) & ~(7<<TXshift));
	*(gpio+fSel) = (*(gpio+fSel) | (2<<TXshift));

	gSel = addrToOffset(AUX_MU_CNTL_REG);
	*(aux+gSel) =  3;
	printf("cntl_reg: %d\n",*(aux + gSel));

	char data = 'a';
	gSel = addrToOffset(AUX_MU_IO_REG);
	while(1)
	{
		*(aux+gSel) = data;
		sleep(1);	
		printf("buff: %c\n",*(aux+gSel));
	}
}

int main(int argc, char* argv[]){
	int fd;
	int baud;
	baud= ((250000000/atoi(argv[1]))/8)-1;
	if((fd = open("/dev/mem",O_RDWR |  O_SYNC))<0){
		printf("Fail open /dev/mem\n");
	}
	gpio = (unsigned int*)mmap(0,BLOCK_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,fd,GPIO_BASE);
	aux = (unsigned int*)mmap(0,BLOCK_SIZE, PROT_READ|PROT_WRITE,MAP_SHARED,fd,AUX_BASE);

	init(baud);
	pin_Mode();
	return 0;
}
