CC = gcc
CFLAGS = -Wall -Wextra -std=c99
SRC = main.c
OUT = order_system
all: $(OUT)
$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)
 clean:
	rm -f $(OUT)
 run: $(OUT)
	./$(OUT)
