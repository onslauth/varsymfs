ifneq ($(KERNELRELEASE),)
VARSYMFS_VERSION="0.1"

EXTRA_CFLAGS += -DVARSYMFS_VERSION=\"$(VARSYMFS_VERSION)\"

obj-m += varsymfs.o

varsymfs-y := inode.o main.o super.o resolve.o environment.o

else

KDIR ?= /lib/modules/`uname -r`/build

default:
	$(MAKE) -C $(KDIR) M=$$PWD

clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean

endif
