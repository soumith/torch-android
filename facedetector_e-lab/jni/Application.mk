# Build only ARMv7-A machine code.
APP_ABI := armeabi armeabi-v7a
APP_STL :=gnustl_static
LOCAL_ARM_NEON := true
APP_OPTIM := debug