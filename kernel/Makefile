obj-m += librefw.o
librefw-objs := src/mod.o src/hooks.o src/state.o src/bogon.o src/utils.o src/nl.o src/hcf.o

ccflags-y += -I$(src)/include

KDIR := /lib/modules/$(shell uname -r)/build

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean
