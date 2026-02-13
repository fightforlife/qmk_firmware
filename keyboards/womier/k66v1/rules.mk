#Custom RGB Matrix driver to combine SN32 and SLED1734
SRC += rgb_matrix.c  
SRC += extras.c  
# Include the patched functions to reduce RAM usage
SRC += custom_drivers/no_idle_thread.c
SRC += custom_drivers/rand.c

# reduce stack size
USE_EXCEPTIONS_STACKSIZE = 0x140
USE_PROCESS_STACKSIZE = 0x1d0
TOP_SYMBOLS = 10000

# Size and optimization flags: LTO, optimize for size, strip unused
EXTRAFLAGS += -flto -Os  -ffunction-sections -fdata-sections -DNO_DEBUG -DNO_PRINT -fstack-usage
LDFLAGS += -Wl,--gc-sections


MOUSE_ENABLE = no
RAW_ENABLE = no
JOYSTICK_ENABLE = no
DIGITIZER_ENABLE = no
VIRTSER_ENABLE = no