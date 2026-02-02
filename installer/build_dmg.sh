#!/bin/bash
set -e

APP_NAME="SHCR"
APP_BUNDLE="app/SHCR.app"
DMG_NAME="SHCR_Installer.dmg"
STAGING="$(pwd)/installer/dmg_staging"

echo "[Installer] Cleaning up..."
rm -rf "$STAGING" "$DMG_NAME"

echo "[Installer] Verifying SHCR.app..."
if [ ! -d "$APP_BUNDLE" ]; then
  echo "Error: SHCR.app not found. Build first."
  exit 1
fi

echo "[Installer] Preparing staging directory..."
mkdir -p "$STAGING"
cp -R "$APP_BUNDLE" "$STAGING/SHCR.app"

echo "[Installer] Copying runtime resources..."
mkdir -p "$STAGING/SHCR.app/Contents/Resources"
cp -R core "$STAGING/SHCR.app/Contents/Resources/core"

echo "[Installer] Creating Applications symlink..."
ln -s /Applications "$STAGING/Applications"

echo "[Installer] Building DMG..."
hdiutil create \
  -volname "SHCR Installer" \
  -srcfolder "$STAGING" \
  -ov \
  -format UDZO \
  "$DMG_NAME"

echo "[Installer] Cleaning staging..."
rm -rf "$STAGING"

echo "✅ DMG created successfully:"
echo "➡️  $(pwd)/$DMG_NAME"
