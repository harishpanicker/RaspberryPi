#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/ioctl.h>
 
#define RD_DATE 	_IOR('a','a',int32_t*)
#define RD_CALENDER 	_IOR('a','b',int32_t*)

int main(){

	int fd,choice,hr,min,sec,day,month,year;
	int32_t value;

	fd = open("/dev/i2c_device",O_RDWR);
	if(fd<0){
		printf("file cannot open\n");
		return 0;
	}

	printf("1) Read hr:min:sec \n");
	printf("2) Read the calender\n");
	printf("Enter the option = ");
	scanf("%d",&choice);

	switch(choice){
		case 1: ioctl(fd, RD_DATE, (int32_t*) &value);
			hr = value/10000;
			value = value%10000;
			min = value/100;
			sec = value%100;
			printf("hr:min:sec = %d:%d:%d\n",hr ,min ,sec);
			break;

		case 2: ioctl(fd, RD_CALENDER, (int32_t*) &value);
			printf("dd:mm:yy = %d:%d:%d\n",day ,month ,year);
			break;

		default: printf("Entered value is wrong..\n");

	}

	close(fd);
	return 0;
}
