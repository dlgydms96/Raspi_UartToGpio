#include<stdio.h>
#include<fcntl.h>
#include<sys/mman.h>
#include<math.h>
///////////////////////
#include<stdint.h>
#include<stdlib.h>
#include<ctype.h>
#include<poll.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<time.h>
#include<pthread.h>
#include<sys/time.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<sys/ioctl.h>
#include<sys/resource.h>
///////////////////////
#define INPUT        0
#define OUTPUT       1
#define LOW		     0
#define HIGH         1

#define BLOCK_SIZE   (4*1024)
#define BCM2708_PERI_BASE     0x3F000000
#define GPIO_BASE    (BCM2708_PERI_BASE + 0x00200000)
#define LoopTimes    620


static volatile unsigned int *gpio;
void pin_25_Mode(int mode);
int digitalRead_pin_25();
void dummyLoop(int microsec);
void forLoops(int times);
int serial2char(int* arr);

int main() 
{
	int which = PRIO_PROCESS;
	id_t pid;
	int priority = 20;
	int ret;

	pid = getpid();
	ret = setpriority(which, pid, priority);
	
	int fd;

	if((fd = open("/dev/mem", O_RDWR | O_SYNC) ) < 0){
		printf("Fail open /dev/mem\n");
	}

	gpio = (unsigned int *)mmap(0, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO_BASE);
	pin_25_Mode(INPUT);
	int buf[8] = {0,0,0,0,0,0,0,0};
	while(1){
		//printf("%d",digitalRead_pin_25());
		ret = setpriority(which, pid, priority);
		if(digitalRead_pin_25() == 0){
			//printf("hello");
			forLoops(LoopTimes);	
			buf[0] = digitalRead_pin_25(); 
			forLoops(LoopTimes);	
			buf[1] = digitalRead_pin_25();
			forLoops(LoopTimes);	
			buf[2] = digitalRead_pin_25(); 
			forLoops(LoopTimes);	
			buf[3] = digitalRead_pin_25();
			forLoops(LoopTimes);	
			buf[4] = digitalRead_pin_25(); 
			forLoops(LoopTimes);	
			buf[5] = digitalRead_pin_25();
			forLoops(LoopTimes);	
			buf[6] = digitalRead_pin_25(); 
			forLoops(LoopTimes);	
			buf[7] = digitalRead_pin_25();
			printf(" Here! : %d %d %d %d %d %d %d %d : end! \n ", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
			printf(" receive %c \n " , serial2char(buf));
		}
		forLoops(LoopTimes);
	}

	return 0;
}

void pin_25_Mode(int mode)
{
	int fSel, shift, alt;

	fSel   = 2;
	shift  = 15;

	if(mode == INPUT)
		*(gpio + fSel) = (*(gpio + fSel) & ~(7 << shift));
	else if(mode == OUTPUT)
		*(gpio + fSel) = (*(gpio + fSel) & ~(7 << shift)) | (1 << shift);
}

int digitalRead_pin_25()
{
	//int gpioToGPLEV = 13;

	if((*(gpio + 13) & (1 <<(25))) != 0)
		return HIGH;
	else
		return LOW;
}

void dummyLoop(int microsec) {
	int loop_count = 70 * microsec;
	for(int i = 0; i < loop_count; i++){ }
}

void forLoops(int times)
{
	for(int i = 0; i < times; i++){}
}

int serial2char(int* arr)
{
	int result = 0;
	int pow_arr[8] = {1, 2, 4, 8, 16, 32, 64, 128};
	for(int i = 0; i < 8; i++)
	{
		result += arr[i] * pow_arr[i];
	}
	return result;
}
