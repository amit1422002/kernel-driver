# Top-level helper. Kernel module still needs KERNEL_DIR.

.PHONY: module userspace clean

module:
	$(MAKE) -C kernel

userspace:
	$(MAKE) -C userspace

clean:
	-$(MAKE) -C kernel clean
	$(MAKE) -C userspace clean
