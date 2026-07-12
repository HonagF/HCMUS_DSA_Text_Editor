CC = gcc
CFLAGS = `pkg-config --cflags gtk4`
LIBS = `pkg-config --libs gtk4`

all: hi

hi: hi.c
	$(CC) hi.c -o hi $(CFLAGS) $(LIBS)

clean:
	rm -f hi