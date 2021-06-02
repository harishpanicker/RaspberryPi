obj-m +=i2c_client_driver.o
obj-m +=i2c_bus_driver.o

KDIR = /lib/modules/$(shell uname -r)/build
all:
	make -C $(KDIR) M=$(shell pwd) modules
clean:
	make -C $(KDIR) M=$(shell pwd) clean
