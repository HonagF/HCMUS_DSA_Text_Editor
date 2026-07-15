CC = gcc
CFLAGS = `pkg-config --cflags gtk4` -g -Wall
LIBS = `pkg-config --libs gtk4`

# Find all .c files in the current directory
SRCS = $(wildcard *.c)
# Define the output executable name
TARGET = notepad

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(SRCS) -o $(TARGET) $(CFLAGS) $(LIBS)

clean:
	rm -f $(TARGET)