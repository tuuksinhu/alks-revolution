CC = gcc
CFLAGS = -O1 -Wall -Wextra -std=c99 -O2

LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

SRC = main.c
OBJ = $(SRC:.c=.o)
TARGET = game

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

run: all
	./$(TARGET)

