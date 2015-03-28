CFLAGS = -c -Wall `pkg-config fuse --cflags --libs`

LDFLAGS = `pkg-config fuse --cflags --libs`

SOURCES = disk_emu.c sfs_api.c fuse_wrappers.c
OBJECTS = $(SOURCES:.c=.o)

TEST_SOURCES = disk_emu.c sfs_api.c sfs_test.c
TEST_OBJECTS = $(TEST_SOURCES:.c=.o)

EXECUTABLE = sfs

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	gcc $(OBJECTS) $(LDFLAGS) -o $@

.c.o:
	gcc $(CFLAGS) $< -o $@

clean:
	rm -rf *.o *~ $(EXECUTABLE)

scp:
	scp -r . root@sudoku.mickey.io:/root/os_sfs

test:
	gcc $(OBJECTS)