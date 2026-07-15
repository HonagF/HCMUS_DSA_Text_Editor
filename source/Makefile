CC = gcc
CFLAGS = $(shell pkg-config --cflags gtk4) -g -Wall
LIBS = $(shell pkg-config --libs gtk4)
SRCS = notepad.c ui.c list.c autocomplete.c undo_redo.c
TARGET = Editor.exe

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(SRCS) -o $(TARGET) $(CFLAGS) $(LIBS)

clean:
	del $(TARGET)
