#!/usr/bin/env bash
# Bootstrap a local Flutter engine tree for the Dual Kawase experiment.
# See docs/engine/impeller/Dual-Kawase-Blur-Experiment.md
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$ROOT"

echo "==> Flutter root: $ROOT"
echo "==> Branch tip: $(git rev-parse --abbrev-ref HEAD) @ $(git rev-parse --short HEAD)"

FREE_GB=$(df -g "$ROOT" 2>/dev/null | awk 'NR==2{print $4}' || df -h "$ROOT" | awk 'NR==2{print $4}')
echo "==> Free disk (approx): $FREE_GB"
echo "    Need roughly ≥80GB free for gclient sync --no-history + a host build."

if ! command -v gclient >/dev/null 2>&1; then
  echo ""
  echo "depot_tools / gclient not on PATH."
  echo "Install:"
  echo "  git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git ~/depot_tools"
  echo "  export PATH=\"\$HOME/depot_tools:\$PATH\""
  exit 1
fi

if [[ ! -f .gclient ]]; then
  echo "==> Writing .gclient from engine/scripts/standard.gclient (fork URL)"
  cat > .gclient <<EOF
solutions = [
  {
    "custom_deps": {},
    "deps_file": "DEPS",
    "managed": False,
    "name": ".",
    "safesync_url": "",
    "url": "git@github.com:definev/flutter.git",
  },
]
EOF
fi

echo "==> Running: gclient sync --no-history"
echo "    (This downloads third_party deps; may take a long time.)"
gclient sync --no-history

echo ""
echo "Next:"
echo "  1. Build host desktop engine (see docs/engine/contributing/Compiling-the-engine.md)"
echo "  2. Wire Dual Kawase pipelines (checklist in Dual-Kawase-Blur-Experiment.md)"
echo "  3. Run with IMPELLER_DUAL_KAWASE_BLUR=1 once the flag hook lands"
echo "  4. Compare BackdropFilter vs stock Gaussian"
