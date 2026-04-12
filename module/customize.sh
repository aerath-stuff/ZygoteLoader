# shellcheck disable=SC2034
SKIPUNZIP=1

NAME=$(grep_prop name "${TMPDIR}/module.prop")
VERSION=$(grep_prop version "${TMPDIR}/module.prop")
ui_print "- Installing $NAME $VERSION"

if [ "$ARCH" != "arm" ] && [ "$ARCH" != "arm64" ] && [ "$ARCH" != "x86" ] && [ "$ARCH" != "x64" ]; then
  abort "! Unsupported platform: $ARCH"
else
  ui_print "- Device platform: $ARCH"
fi

abort_verify() {
  ui_print "***********************************************************"
  ui_print "! $1"
  ui_print "! This zip is incomplete"
  abort    "***********************************************************"
}

extract() {
  local zip="$1"
  local target="$2"
  local dir="$3"
  local junk_paths="${4:-false}"
  local opts="-o"
  local target_path

  [[ "$junk_paths" == true ]] && opts="-oj"

  if [[ "$target" == */ ]]; then
    target_path="$dir/$(basename "$target")"
    unzip $opts "$zip" "${target}*" -d "$dir" >&2
    [[ -d "$target_path" ]] || abort_verify "$target directory doesn't exist"
  else
    target_path="$dir/$(basename "$file")"
    unzip $opts "$zip" "$target" -d "$dir" >&2
    [[ -f "$target_path" || -d "$target_path" ]] || abort_verify "$target file doesn't exist"
  fi
}

ui_print "- Extracting module files"
extract "$ZIPFILE" 'module.prop'     "$MODPATH"

if [ "$ARCH" = "x86" ] || [ "$ARCH" = "x64" ]; then
  ui_print "- Extracting x86 libraries"

  extract "$ZIPFILE" 'zygisk/x86.so' "$MODPATH/zygisk" true
  mv "$MODPATH/zygisk/x86.so" "$MODPATH/zygisk/x86.so"

  ui_print "- Extracting x64 libraries"

  extract "$ZIPFILE" 'zygisk/x64.so' "$MODPATH/zygisk" true
  mv "$MODPATH/zygisk/x64.so" "$MODPATH/zygisk/x86_64.so"
else
  ui_print "- Extracting arm libraries"

  extract "$ZIPFILE" 'zygisk/armeabi-v7a.so' "$MODPATH/zygisk" true
  mv "$MODPATH/zygisk/armeabi-v7a.so" "$MODPATH/zygisk/armeabi-v7a.so"

  ui_print "- Extracting arm64 libraries"

  extract "$ZIPFILE" 'zygisk/arm64-v8a.so' "$MODPATH/zygisk" true
  mv "$MODPATH/zygisk/arm64-v8a.so" "$MODPATH/zygisk/arm64-v8a.so"
fi

ui_print "- Welcome to $NAME $VERSION"
