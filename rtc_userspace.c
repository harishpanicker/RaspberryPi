#include<stdio.h>
#include<stdint.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/ioctl.h>

#define RD_TIME 	_IOR('a','a',int32_t*)
#define RD_DATE 	_IOR('a','b',int32_t*)
#define RD_DAY	 	_IOR('a','c',uint8_t*)

int main(){

	int fd,choice,hr,min,sec,dd,mm,yy;
	int32_t value;
	uint8_t day[10];

	fd = open("/dev/i2c_device",O_RDWR);
	if(fd<0){
		printf("file cannot open\n");
		return 0;
	}

	printf("1) Get hr:min:sec \n");
	printf("2) Get the calender\n");
	printf("3) Get the day\n");
	printf("Enter the option = ");
	scanf("%d",&choice);

	switch(choice){
		case 1: ioctl(fd, RD_TIME, (int32_t*) &value);
			hr = value/10000;
			value = value%10000;
			min = value/100;
			sec = value%100;
			printf("hr:min:sec = %x:%x:%x\n",hr ,min ,sec);
			break;

		case 2: ioctl(fd, RD_DATE, (int32_t*) &value);
			dd = value/10000;
			value = value%10000;
			mm = value/100;
			yy = value%100;
			if(yy<10)
			printf("dd:mm:yy = %x:%x:200%x\n",dd ,mm ,yy);
			else
			printf("dd:mm:yy = %x:%x:20%x\n",dd ,mm ,yy);
			break;
		case 3: ioctl(fd, RD_DAY, (uint8_t*) day);
			printf("day = %s\n",day);
			break;
		default: printf("Entered value is wrong..\n");

	}

	close(fd);
	return 0;
}
