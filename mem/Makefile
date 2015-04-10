CFLAGS = -c -Wall

SOURCES = my_malloc.c test.c
OBJECTS = $(SOURCES:.c=.o)

EXECUTABLE = test

all: $(SOURCES) $(EXECUTABLE)
	./$(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	gcc $(OBJECTS) -o $@

%.o: %.c
	gcc $(CFLAGS) $< -o $@

clean:
	rm -rf *.o *~ $(EXECUTABLE)

scp:
	scp -r . root@sudoku.mickey.io:/root/os_mem