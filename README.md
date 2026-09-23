# Android Kernel Driver (akd)

Out-of-tree **misc character device** for Android kernels. It creates `/dev/akd`
with read/write and a small ioctl set, plus a userspace tool (`akdctl`) to
exercise it.

This is a bring-up / learning driver, not a hardware-specific SoC driver.

## Layout

```
kernel/         akd.c, akd.h, Makefile, Kconfig, Android.bp
userspace/      akdctl test tool
sepolicy/       SELinux types and file_contexts
init/           ueventd permissions
```

## What the driver does

| Interface | Behavior |
|-----------|----------|
| `write` | Stores up to 4096 bytes in a kernel buffer |
| `read` | Returns that buffer |
| `AKD_IOC_GET_VERSION` | Driver version (`1.0.0`) |
| `AKD_IOC_GET_STATUS` / `SET_STATUS` | Idle / busy / error |
| `AKD_IOC_CLEAR` | Zero the buffer |
| `AKD_IOC_GET_LEN` | Current stored length |

## Prerequisites

- Android kernel source that **matches the device** (same branch / GKI version)
- Configured kernel (`make ... defconfig` already done; `Module.symvers` present)
- Cross toolchain for the kernel ABI, usually `aarch64-linux-gnu-` or the
  Android prebuilt clang/gcc from AOSP
- Device with **unlocked bootloader** if you load a custom module, or an
  emulator / cuttlefish / hikey board you control
- Root (or a properly labeled vendor domain) to `insmod` and open `/dev/akd`

Modern GKI devices often **reject unsigned out-of-tree modules**. Options:

1. Build the module **in-tree** as part of your kernel / vendor_boot
2. Use a device that allows `CONFIG_MODULE_SIG=n` (emulator, older trees)
3. Sign the `.ko` with the kernel’s module key

## Build the kernel module (standalone)

```bash
export KERNEL_DIR=/path/to/android/kernel
export ARCH=arm64
export CROSS_COMPILE=aarch64-linux-gnu-

cd kernel
make
```

You should get `akd.ko`.

Clang (typical AOSP):

```bash
cd $KERNEL_DIR
make LLVM=1 ARCH=arm64 M=/path/to/kerne\ driver/kernel modules
```

## Build akdctl

On a Linux host targeting the device:

```bash
cd userspace
make CC=aarch64-linux-gnu-gcc
```

Or add `userspace/Android.bp` to a vendor project and build with Soong:

```bash
m akdctl
```

## Load on device

```bash
adb root
adb remount
adb push kernel/akd.ko /data/local/tmp/
adb push userspace/akdctl /data/local/tmp/
adb shell
insmod /data/local/tmp/akd.ko debug=1
ls -l /dev/akd
dmesg | grep akd

/data/local/tmp/akdctl version
/data/local/tmp/akdctl write hello
/data/local/tmp/akdctl read
/data/local/tmp/akdctl status
/data/local/tmp/akdctl set-status 1
/data/local/tmp/akdctl clear
rmmod akd
```

If `insmod` fails with **Exec format error**, the kernel ABI does not match
(wrong `KERNEL_DIR`, wrong arch, or GKI version mismatch).

If open fails with **Permission denied**, fix `ueventd` permissions and
SELinux (below), or for a quick eng test:

```bash
chmod 666 /dev/akd
setenforce 0    # userdebug/eng only
```

## In-tree / AOSP integration

1. Copy `kernel/` into your kernel tree, e.g. `drivers/misc/akd/`.
2. Add to that directory’s `Makefile`:

   ```
   obj-$(CONFIG_AKD) += akd.o
   ```

3. Source `Kconfig` from `drivers/misc/Kconfig`.
4. Enable `CONFIG_AKD=m` (or `=y`) in the device defconfig.
5. Copy `sepolicy/` into the device vendor sepolicy.
6. Merge `init/ueventd.akd.rc` into the device `ueventd` fragment.
7. Install `akdctl` as a vendor binary via `userspace/Android.bp`.

## Notes

- License is **GPL-2.0** (`MODULE_LICENSE("GPL")`) so the module can use
  exported GPL kernel symbols.
- The driver uses `misc_register()`, so the major is `10` and the minor is
  allocated dynamically. No `mknod` is needed; udev/ueventd creates `/dev/akd`.
- Do not load this on a production phone you do not own. Custom kernel
  modules can brick a device if the ABI is wrong.

## Next steps

Hardware-backed drivers usually add:

- A **platform_driver** + Device Tree `compatible` string
- `probe` / `remove` instead of only `module_init`
- `regmap` / `ioremap` for MMIO
- `irq` handlers and runtime PM
- A binding document under `Documentation/devicetree/bindings/`
