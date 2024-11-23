#Custom RGB Matrix driver to combine SN32 and SLED1734
SRC += rgb_matrix.c  
# Include the patched functions to reduce RAM usage
SRC += custom_drivers/no_idle_thread.c
SRC += custom_drivers/rand.c

# reduce stack size
USE_EXCEPTIONS_STACKSIZE = 0x150
#0x135 OK #0x100 NOK 105 OK/ 0x125 OK
USE_PROCESS_STACKSIZE = 0x200
# 0x195 OK #0x150 NOK 155 OK / 0x175 OK
TOP_SYMBOLS = yes

#EXTRAFLAGS += -flto -O3 #more RAM with O3