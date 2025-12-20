#Custom RGB Matrix driver to combine SN32 and SLED1734
SRC += rgb_matrix.c  
# Include the patched functions to reduce RAM usage
SRC += custom_drivers/no_idle_thread.c
SRC += custom_drivers/rand.c

# reduce stack size
USE_EXCEPTIONS_STACKSIZE = 0x130
# 0x118 last OK
USE_PROCESS_STACKSIZE = 0x1c0
# 0x1a0 last ok
TOP_SYMBOLS = 10000

EXTRAFLAGS += -flto -Os#-O3 #more RAM with O3