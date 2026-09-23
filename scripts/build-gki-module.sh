#!/usr/bin/env bash
# Build akd.ko for this POCO X6 Pro (duchamp) GKI kernel:
#   6.1.138-android14-11-g44bda9e8f6e9-ab13792638
#
# Run on Linux or WSL (Ubuntu). Not on native Windows.
#
#   chmod +x scripts/build-gki-module.sh
#   ./scripts/build-gki-module.sh
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
WORKDIR="${WORKDIR:-$ROOT/.gki-build}"
BID="${BID:-13792638}"
COMMON_BRANCH="${COMMON_BRANCH:-android14-6.1}"
CLANG_VERSION="${CLANG_VERSION:-r487747c}"

echo "Work dir: $WORKDIR"
mkdir -p "$WORKDIR"
cd "$WORKDIR"

if ! command -v git >/dev/null; then
	echo "git required" >&2
	exit 1
fi

if [[ ! -d common/.git ]]; then
	echo "Cloning GKI common ($COMMON_BRANCH)..."
	git clone --depth 1 -b "$COMMON_BRANCH" \
		https://android.googlesource.com/kernel/common common
else
	echo "Reusing $WORKDIR/common"
fi

# Prebuilt Android clang (same family as the phone: clang 17.0.2 / r487747c)
if [[ ! -x clang/bin/clang ]]; then
	echo "You need Android clang $CLANG_VERSION on PATH or extracted to $WORKDIR/clang"
	echo "AOSP prebuilt:"
	echo "  git clone --depth 1 https://android.googlesource.com/platform/prebuilts/clang/host/linux-x86 clang-host"
	echo "  then point CLANG_DIR=.../clang-$CLANG_VERSION"
	exit 1
fi

export ARCH=arm64
export LLVM=1
export PATH="$WORKDIR/clang/bin:$PATH"

echo "Preparing GKI headers (modules_prepare)..."
cd "$WORKDIR/common"
make LLVM=1 ARCH=arm64 gki_defconfig
make LLVM=1 ARCH=arm64 modules_prepare

echo "Building akd.ko..."
make LLVM=1 ARCH=arm64 M="$ROOT/kernel" modules

echo
echo "Done: $ROOT/kernel/akd.ko"
echo "Push + load:"
echo "  adb push $ROOT/kernel/akd.ko /data/local/tmp/"
echo "  adb shell su -c '/data/adb/ksu/bin/ksud insmod /data/local/tmp/akd.ko debug=1'"
