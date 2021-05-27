#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define SET_GPIO_PIN _IO('a','a') 
#define CLEAR_GPIO_PIN _IO('a','b')
#define READ_GPIO_PIN _IOR('a','c',int32_t*)

int main()
{
	int fd;
	char option;
	int value = 0;
	fd = open("/dev/gpio_pin",O_RDWR);
	if(fd <0)
	{
		printf("can not open the device");
		return 0;
	}

	while(1)
	{
		printf("perform below IOCTL operations:\n");
		printf(" 	1. Set GPIO pin		\n");
		printf(" 	2. Clear GPIO pin		\n");
		printf(" 	3. Read GPIO pin		\n");
		printf(" 	4. Back to main menu		\n");

		printf("Enter the number to perform IOCTL operations:\n");
		scanf(" %c",&option);
		switch(option)
		{
			case '1':
				printf("Setting the GPIO pin...\n\n");
				ioctl(fd, SET_GPIO_PIN);
				break;
			case '2':
				printf("Clearing the GPIO pin...\n\n");
		 		ioctl(fd, CLEAR_GPIO_PIN);
				break;
			case '3':
				printf("Reading the GPIO pin...\n\n");
		 		ioctl(fd, READ_GPIO_PIN,(int32_t*)&value);
        			printf("GPIO PIN =  %d\n", value);
				break;
			case '4':
				close(fd);
				exit(EXIT_SUCCESS);			
			default:
		   		printf("Enter Valid option between 1 - 4\n");
		   		break;
		}
	
	}

return 0;
}
