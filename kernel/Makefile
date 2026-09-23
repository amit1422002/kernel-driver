# Out-of-tree Android / Linux kernel module
#
# Usage:
#   make KERNEL_DIR=/path/to/android/kernel ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu-
#
# KERNEL_DIR must point at a configured/built kernel tree (or GKI common kernel).

obj-m += akd.o

KERNEL_DIR ?=
ARCH ?= arm64
CROSS_COMPILE ?= aarch64-linux-gnu-

ifeq ($(KERNEL_DIR),)
$(warning KERNEL_DIR is not set. Example:)
$(warning   make KERNEL_DIR=/path/to/kernel ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu-)
endif

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) clean
	rm -f *.o *.ko *.mod.c *.mod .*.cmd Module.symvers modules.order

.PHONY: all clean
