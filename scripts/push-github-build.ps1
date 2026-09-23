# One-time: gh auth login
# Then from project root:
#   git init -b main
#   git add -A
#   git commit --trailer "Co-authored-by: Cursor <cursoragent@cursor.com>" -m "Add akd driver and CI build"
#   gh repo create kerne-driver-akd --public --source=. --push
#   gh workflow run build-akd-module.yml
#   gh run watch
#   gh run download -n akd-ko-arm64

Write-Host "See BUILD-AKD.md"
