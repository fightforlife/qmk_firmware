#Custom RGB Matrix driver to combine SN32 and SLED1734
# SRC = rgb_matrix.c  
# Include the patched functions to reduce RAM usage
SRC += drivers/no_idle_thread.c
SRC += drivers/rand.c

# reduce stack size
USE_EXCEPTIONS_STACKSIZE = 0x125
#0x135 OK #0x100 NOK 105 OK
USE_PROCESS_STACKSIZE = 0x175
# 0x195 OK #0x150 NOK 155 OK
TOP_SYMBOLS = yes