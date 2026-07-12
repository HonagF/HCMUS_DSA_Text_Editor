CC = gcc
CFLAGS = -Wall -g
TARGET = TestList.exe
SRCS = list.c test_list.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	del $(TARGET) 
