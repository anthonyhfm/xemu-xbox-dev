#!/bin/bash

set -eu

dir="$1"
XEMU_DATE=$(date -u)
if test -e "$dir/.git"; then
  XEMU_COMMIT=$(git -C "$dir" rev-parse HEAD 2>/dev/null | tr -d '\n') || true
elif test -e "$dir/XEMU_COMMIT"; then
  XEMU_COMMIT=$(cat "$dir/XEMU_COMMIT") || true
else
  XEMU_COMMIT=""
fi
if test -e "$dir/.git"; then
  XEMU_VERSION=$(git -C "$dir" describe --tags --match 'v*' 2>/dev/null | cut -c 2- | tr -d '\n') || true
elif test -e "$dir/XEMU_VERSION"; then
  XEMU_VERSION=$(cat "$dir/XEMU_VERSION") || true
else
  XEMU_VERSION=""
fi

if [ -z "${XEMU_VERSION}" ]; then
  XEMU_VERSION="0.0.0"
fi

get_version_field() {
  echo ${XEMU_VERSION}-0 | cut -d- -f$1
}

get_version_dot () {
  echo $(get_version_field 1) | cut -d. -f$1
}

XEMU_VERSION_MAJOR=$(get_version_dot 1)
XEMU_VERSION_MINOR=$(get_version_dot 2)
XEMU_VERSION_PATCH=$(get_version_dot 3)
XEMU_VERSION_COMMIT=$(get_version_field 2)

cat <<EOF
#define XEMU_VERSION       "$XEMU_VERSION"
#define XEMU_VERSION_MAJOR $XEMU_VERSION_MAJOR
#define XEMU_VERSION_MINOR $XEMU_VERSION_MINOR
#define XEMU_VERSION_PATCH $XEMU_VERSION_PATCH
#define XEMU_VERSION_COMMIT $XEMU_VERSION_COMMIT
#define XEMU_COMMIT        "$XEMU_COMMIT"
#define XEMU_DATE          "$XEMU_DATE"
EOF
