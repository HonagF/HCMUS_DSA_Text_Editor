CC = gcc
CFLAGS = -Wall -g
TARGET = Editor.exe
SRCS = list.c main.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	del $(TARGET) 
