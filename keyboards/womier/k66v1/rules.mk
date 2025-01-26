#Custom RGB Matrix driver to combine SN32 and SLED1734
SRC += rgb_matrix.c  
# Include the patched functions to reduce RAM usage
SRC += custom_drivers/no_idle_thread.c
SRC += custom_drivers/rand.c

# reduce stack size
USE_EXCEPTIONS_STACKSIZE = 0x118
# 0x140 OK 0x100
USE_PROCESS_STACKSIZE = 0x1a0
# 0x190 OK 0z180
TOP_SYMBOLS = 20

EXTRAFLAGS += -flto -Os#-O3 #more RAM with O3
