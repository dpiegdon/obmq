# Makefile for obmq ledtrigger driver for linux kernel

ifneq ($(KERNELRELEASE),)

        PWD := $(shell pwd)
        obj-m := ledtrig-obmq.o
	ledtrig-obmq-y := ledtrig.o obmq.o

else

.PHONY: all clean modules_install

        KERNEL_SRC ?= /lib/modules/$(shell uname -r)/build
        PWD := $(shell pwd)

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) clean

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules_install

endif

