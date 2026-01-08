CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99
TARGET = earlyfreeze
SRC = earlyfreeze.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

install: all
	install -m 755 $(TARGET) /usr/local/bin/

uninstall:
	rm -f /usr/local/bin/$(TARGET)
