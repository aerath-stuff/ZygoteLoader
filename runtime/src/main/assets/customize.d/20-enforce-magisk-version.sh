# Enforce Magisk version, skip for KernelSU or APatch
# Supported Magisk version: 24000+

if [ "$KSU" ]
then
    ui_print "- KernelSU version: $KSU_VER (ksud: $KSU_VER_CODE - kernel: $KSU_KERNEL_VER_CODE)"
elif [ "$APATCH" ]
then
    ui_print "- APatch version: $APATCH_VER ($APATCH_VER_CODE)"
else
    [ "$MAGISK_VER_CODE" -lt 24000 ] && abort "! Unsupported magisk version: $MAGISK_VER_CODE (24000+ required)"

    ui_print "- Magisk version: $MAGISK_VER ($MAGISK_VER_CODE)"
fi
