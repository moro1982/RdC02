CC = gcc
CFLAGS = -Wall -Wextra -std=gnu11 -g
LDFLAGS = 

SRCS = main.c server_ftp.c arguments.c pi.c dtp.c handlers.c
OBJS = $(SRCS:.c=.o)
TARGET = myftp

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(FLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)