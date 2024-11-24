#Custom RGB Matrix driver to combine SN32 and SLED1734
SRC += rgb_matrix.c  
# Include the patched functions to reduce RAM usage
SRC += custom_drivers/no_idle_thread.c
SRC += custom_drivers/rand.c

# reduce stack size
USE_EXCEPTIONS_STACKSIZE = 0x100
# 0x140 OK 0x100
USE_PROCESS_STACKSIZE = 0x180
# 0x190 OK 0z180
TOP_SYMBOLS = yes

#EXTRAFLAGS += -flto -O3 #more RAM with O3