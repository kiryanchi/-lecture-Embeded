obj-m = gpioirq_module.obj

KDIR=/lib/module/$(shell uname -r)/build
PWD:=$(shell pwd)

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	rm -rf *.o *.ko *.mod.c .led.* Module.symvers modules.order