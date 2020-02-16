CC=gcc
CFLAGS=-g -W
LDFLAGS=-lpthread
TARGET=reclocking-analysis
SRC=main.c avx_256.S avx_512.S exec_exit.S
OBJ_C=$(SRC:.c=.c.o)
OBJ=$(OBJ_C:.S=.S.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

%.c.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.S.o: %.S
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean
