# Fastest way to get `akd.ko` (no local Linux)

Your PC has **no WSL yet** (reboot may be needed after `install-wsl.ps1`). Use **GitHub Actions** (~10–20 min).

## Steps

1. Install GitHub CLI if needed: `winget install GitHub.cli`

2. Login (browser once):

   ```powershell
   gh auth login
   ```

3. From `E:\kerne driver`:

   ```powershell
   git init -b main
   git add -A
   git commit -m "Add akd driver and CI build"
   gh repo create kerne-driver-akd --public --source=. --push
   gh workflow run build-akd-module.yml
   gh run watch
   gh run download -n akd-ko-arm64
   ```

4. Load on phone:

   ```powershell
   adb push akd.ko /data/local/tmp/akd.ko
   adb shell "su -c '/data/adb/ksu/bin/ksud insmod /data/local/tmp/akd.ko debug=1'"
   adb shell "su -c 'ls -l /dev/akd'"
   ```

## After WSL works (faster next builds)

Reboot if DISM asked (3010), then:

```powershell
wsl --install -d Ubuntu --web-download
```

In Ubuntu:

```bash
cd /mnt/e/kerne\ driver
bash scripts/wsl-build-akd.sh
```

## Note

Vermagic must match your phone (`6.1.138-android14-11-maybe-dirty`). CI uses `android14-6.1` branch; if `insmod` fails with **Invalid module format**, we need a kernel tree pinned to your exact build `13792638`.
