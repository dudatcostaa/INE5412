CC = gcc
CFLAGS = -Wall -Wextra -std=c11

TARGET = scheduler
SRC = escalonador.c escalonador_io.c
HDR = escalonador_io.h

all: $(TARGET)

$(TARGET): $(SRC) $(HDR)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

zip: clean
	cd .. && zip -r t1-so-solucao.zip t1-so "t1-so/"

.PHONY: all clean zip
