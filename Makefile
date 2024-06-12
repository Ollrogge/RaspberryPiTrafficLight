ARCH=arm64
CROSS_COMPILE=aarch64-linux-gnu-
obj-m += traffic_light.o

KERNEL_DIR := /home/h0ps/Programming/rpi3_embedded/linux
PWD := $(shell pwd)

# sudo apt install raspberrypi-kernel-headers
# ln -s /usr/src/linux-headers-6.6.31+rpt-rpi-v8/ /lib/modules/6.6.31+rpt-rpi-v8/build

all:
	# make -C $(KERNEL_DIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	# make -C $(KERNEL_DIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) clean
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

