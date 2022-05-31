#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdint.h>
#include<sys/ioctl.h>

#define RD_CHIP_VALUE _IOR('a','a',int32_t*)
#define RD_TEMP_VALUE _IOR('a','b',uint8_t*)
#define RD_PRESS_VALUE _IOR('a','c',uint8_t*)

int main(){

        int fd,choice;
        int32_t chip_ID;
        uint8_t Temp_value[1024],Press_value[1024];

        fd = open("/dev/i2c_device",O_RDWR);
        if(fd<0){
                printf("file cannot open\n");
                return 0;
        }

        printf("1) Read chip ID\n");
        printf("2) Read the temperature value\n");
        printf("3) Read the pressure value\n");
        printf("Enter the option = ");
        scanf("%d",&choice);

        switch(choice){
                case 1: ioctl(fd, RD_CHIP_VALUE, (int32_t*) &chip_ID);
                        printf("Chip Id = 0x%x\n",chip_ID);
                        break;

                case 2: ioctl(fd, RD_TEMP_VALUE, (uint8_t*) Temp_value);
                        printf("Temperature Value = %s\n",Temp_value);
                        break;

                case 3: ioctl(fd, RD_PRESS_VALUE, (uint8_t*) Press_value);
                        printf("Pressure Value = %s\n",Press_value);
                        break;
                default: printf("Entered value is wrong..\n");

        }



        return 0;
}

