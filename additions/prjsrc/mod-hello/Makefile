obj-m += mod-hello.o

PWD := $(shell pwd)

all:
	make -C $(KERNEL_LOCATION) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

modules_install:
	make -C $(KERNEL_LOCATION) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules_install

clean:
	make -C $(KERNEL_LOCATION) M=$(PWD) clean

