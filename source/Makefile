CC = gcc
CFLAGS = -Wall -g
TARGET = Editor.exe
SRCS = list.c main.c autocomplete.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(SRCS) -o $(TARGET) $(CFLAGS)

clean:
	del $(TARGET)
