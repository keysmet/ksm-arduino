#!/usr/bin/env bash
# build-arduino-package.sh
# Packages the Arduino board support files for the Keysmet nRF52 platform.
#
# Usage: bash tools/build-arduino-package.sh [version]
#   version defaults to 1.0.0
#
# Output:
#   keysmet-nrf52-<version>.tar.bz2   (upload this to the GitHub Release)
#   package_keysmet_index.json         (updated in-place with checksum + size)

set -e

VERSION="${1:-1.0.0}"
ARCHIVE_NAME="keysmet-nrf52-${VERSION}"
ARCHIVE_FILE="${ARCHIVE_NAME}.tar.bz2"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
SRC_DIR="${REPO_ROOT}/arduino/keysmet"
VARIANTS_DIR="${REPO_ROOT}/variants"
INDEX_FILE="${REPO_ROOT}/package_keysmet_index.json"

echo "==> Building ${ARCHIVE_FILE} from ${SRC_DIR}"

# ── Create archive ─────────────────────────────────────────────────────────
# Stage into a temp dir so the archive unpacks as keysmet-nrf52-<version>/
TMPDIR="$(mktemp -d)"
trap 'rm -rf "${TMPDIR}"' EXIT

cp -r "${SRC_DIR}" "${TMPDIR}/${ARCHIVE_NAME}"

# The variant is NOT checked in under arduino/ — variants/ is the single
# source of truth, shared with PlatformIO via board_build.variants_dir in
# platformio.ini. Copy it in at package time so the two can never drift.
mkdir -p "${TMPDIR}/${ARCHIVE_NAME}/variants"
cp -r "${VARIANTS_DIR}"/* "${TMPDIR}/${ARCHIVE_NAME}/variants/"
echo "==> Bundled variants from ${VARIANTS_DIR}"

tar -C "${TMPDIR}" -cjf "${REPO_ROOT}/${ARCHIVE_FILE}" "${ARCHIVE_NAME}"

echo "==> Created ${REPO_ROOT}/${ARCHIVE_FILE}"

# ── Compute checksum and size ──────────────────────────────────────────────
cd "${REPO_ROOT}"

if command -v sha256sum &>/dev/null; then
    CHECKSUM="SHA-256:$(sha256sum "${ARCHIVE_FILE}" | awk '{print $1}')"
elif command -v shasum &>/dev/null; then
    CHECKSUM="SHA-256:$(shasum -a 256 "${ARCHIVE_FILE}" | awk '{print $1}')"
else
    echo "ERROR: no sha256sum or shasum found" >&2
    exit 1
fi

SIZE=$(wc -c < "${ARCHIVE_FILE}" | tr -d ' ')

echo "==> Checksum : ${CHECKSUM}"
echo "==> Size     : ${SIZE} bytes"

# ── Patch package_keysmet_index.json ──────────────────────────────────────
# Requires python3 (available on all platforms)
python - "${INDEX_FILE}" "${VERSION}" "${CHECKSUM}" "${SIZE}" "${ARCHIVE_FILE}" <<'EOF'
import json, sys

index_path, version, checksum, size, archive_file = sys.argv[1:]

with open(index_path) as f:
    data = json.load(f)

updated = False
for pkg in data["packages"]:
    for platform in pkg["platforms"]:
        if platform["version"] == version:
            platform["checksum"] = checksum
            platform["size"] = size
            platform["archiveFileName"] = archive_file
            updated = True

if not updated:
    print(f"WARNING: no platform entry with version {version} found in index", file=sys.stderr)

with open(index_path, "w") as f:
    json.dump(data, f, indent=2)
    f.write("\n")

print(f"==> Updated {index_path}")
EOF

echo ""
echo "Done! Next steps:"
echo "  1. Commit the updated package_keysmet_index.json"
echo "  2. Create a GitHub Release tagged v${VERSION}"
echo "  3. Upload ${ARCHIVE_FILE} as a release asset"
echo "  4. Host package_keysmet_index.json at a public URL, e.g.:"
echo "     https://raw.githubusercontent.com/keysmet/ksm-arduino/main/package_keysmet_index.json"
echo "  5. In Arduino IDE: File > Preferences > Additional boards manager URLs"
echo "     paste the URL above, then install 'Keysmet nRF52 Boards' from Boards Manager"
