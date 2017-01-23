CFLAGS += -Wall -g
CFLAGS += -I/usr/local/include/
CFLAGS += -std=gnu11 -D_GNU_SOURCE
LDFLAGS += -lpthread

TARGET_C := $(shell ls *.c)
TARGET_OBJ = $(TARGET_C:.c=.o)
TARGET = $(subst .c,,$(TARGET_C))

all: $(TARGET_OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET_C) $(LDFLAGS)

%.o: %.c *.h
	$(CC) $(CFLAGS) -c $@ $< $(LDFLAGS)

clean:
	rm -f *.o $(TARGET)
