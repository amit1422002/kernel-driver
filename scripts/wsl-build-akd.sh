#!/usr/bin/env bash
# Run inside WSL. Builds akd.ko for POCO X6 Pro GKI 6.1.138-android14-11.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
# Prefer E: so C: (16GB free) is not filled
WORKDIR="${WORKDIR:-/mnt/e/kerne-driver-build}"
JOBS="$(nproc)"

echo "ROOT=$ROOT"
echo "WORKDIR=$WORKDIR"
mkdir -p "$WORKDIR"
cd "$WORKDIR"

export DEBIAN_FRONTEND=noninteractive
sudo apt-get update
sudo apt-get install -y --no-install-recommends \
	build-essential flex bison libssl-dev libelf-dev bc git \
	ca-certificates curl wget python3 \
	clang lld llvm \
	gcc-aarch64-linux-gnu

# Shallow clone of the Android 14 GKI 6.1 common kernel
if [[ ! -d common/.git ]]; then
	echo "Cloning kernel/common android14-6.1 ..."
	git clone --depth 1 --branch android14-6.1 \
		https://android.googlesource.com/kernel/common common
else
	echo "Reusing $WORKDIR/common"
fi

# Phone reports 6.1.138 — try to land on that version if this clone is newer
cd "$WORKDIR/common"
if [[ -f Makefile ]]; then
	echo "Kernel Makefile version:"
	head -n 5 Makefile
fi

# Use Ubuntu clang (17/18) via LLVM=1. Close enough for a misc char driver.
export ARCH=arm64
export LLVM=1
export KCFLAGS="-Wno-error"

echo "Configuring gki_defconfig..."
make LLVM=1 ARCH=arm64 gki_defconfig

# Align LOCALVERSION with the running phone as much as possible
# vermagic on device: 6.1.138-android14-11-maybe-dirty SMP preempt mod_unload modversions aarch64
scripts/config --set-str LOCALVERSION "-android14-11-maybe-dirty" || true
scripts/config --enable MODVERSIONS || true
scripts/config --enable MODULES || true
scripts/config --disable MODULE_SIG_FORCE || true

echo "modules_prepare..."
make LLVM=1 ARCH=arm64 modules_prepare -j"$JOBS"

echo "Building akd.ko..."
make LLVM=1 ARCH=arm64 M="$ROOT/kernel" modules -j"$JOBS"

ls -l "$ROOT/kernel/akd.ko"
echo "FILE=$ROOT/kernel/akd.ko"
