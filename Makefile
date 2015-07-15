OBJS = clock.o i2c.o usart.o

BINARY = i2c-scan

# we use sin/cos from the library
LDLIBS += -lm

LDSCRIPT = ../stm32f429i-discovery.ld

include Makefile.include
