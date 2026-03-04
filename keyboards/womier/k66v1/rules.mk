# Custom code
SRC += lib/core/extra.c
SRC += lib/core/no_idle_thread.c
SRC += lib/core/rand.c
SRC += lib/eeprom/eeprom_sn32_legacy.c
SRC += lib/flash/sn32_legacy_flash_ops.c
SRC += rgb_matrix.c  


# reduce stack size
USE_EXCEPTIONS_STACKSIZE = 0x130
USE_PROCESS_STACKSIZE = 0x1D0
TOP_SYMBOLS = 10000

# Size and optimization flags: LTO, optimize for size, strip unused
EXTRAFLAGS += -flto -Os  -ffunction-sections -fdata-sections -DNO_DEBUG -DNO_PRINT -fstack-usage
LDFLAGS += -Wl,--gc-sections,-Map,output.map


MOUSE_ENABLE = no
RAW_ENABLE = no
JOYSTICK_ENABLE = no
DIGITIZER_ENABLE = no
VIRTSER_ENABLE = no

SHARED_EP_ENABLE = yes
KEYBOARD_SHARED_EP = yes