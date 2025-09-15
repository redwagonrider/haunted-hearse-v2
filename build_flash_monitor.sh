#!/bin/bash

# Set project directory (optional: override with $1)
PROJECT_DIR="${1:-$(pwd)}"

# Navigate to project directory
cd "$PROJECT_DIR" || {
  echo "❌ Error: Failed to enter project directory: $PROJECT_DIR"
  exit 1
}

echo "🚧 Cleaning previous builds..."
platformio run --target clean || {
  echo "❌ Build clean failed."
  exit 1
}

echo "��️ Building firmware for Mega 2560..."
platformio run --environment mega2560 || {
  echo "❌ Build failed."
  exit 1
}

echo "⬆️ Uploading to Mega 2560..."
platformio run --target upload --environment mega2560 || {
  echo "❌ Upload failed."
  exit 1
}

echo "📟 Opening Serial Monitor..."
platformio device monitor --environment mega2560#!/bin/bash

# Set project directory (optional: override with $1)
PROJECT_DIR="${1:-$(pwd)}"

# Navigate to project directory
cd "$PROJECT_DIR" || {
  echo "❌ Error: Failed to enter project directory: $PROJECT_DIR"
  exit 1
}

echo "🚧 Cleaning previous builds..."
platformio run --target clean || {
  echo "❌ Build clean failed."
  exit 1
}

echo "��️ Building firmware for Mega 2560..."
platformio run --environment mega2560 || {
  echo "❌ Build failed."
  exit 1
}

echo "⬆️ Uploading to Mega 2560..."
platformio run --target upload --environment mega2560 || {
  echo "❌ Upload failed."
  exit 1
}

echo "📟 Opening Serial Monitor..."
platformio device monitor --environment mega2560
