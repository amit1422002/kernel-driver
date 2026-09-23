# Elevated WSL + Ubuntu bootstrap. Safe to re-run.
$ErrorActionPreference = 'Continue'
$log = 'E:\kerne driver\scripts\install-wsl.log'
function Log($m) {
    $line = "$(Get-Date -Format o)  $m"
    Add-Content -Path $log -Value $line
    Write-Host $line
}

Log '=== enable Windows features ==='
dism.exe /online /enable-feature /featurename:Microsoft-Windows-Subsystem-Linux /all /norestart
Log "dism WSL exit=$LASTEXITCODE"
dism.exe /online /enable-feature /featurename:VirtualMachinePlatform /all /norestart
Log "dism VM platform exit=$LASTEXITCODE"

Log '=== wsl --install ==='
& wsl.exe --install -d Ubuntu --web-download --no-launch
Log "wsl install exit=$LASTEXITCODE"

Log '=== wsl --update ==='
& wsl.exe --update --web-download
Log "wsl update exit=$LASTEXITCODE"

Log '=== wsl -l -v ==='
& wsl.exe -l -v
Log 'done'
