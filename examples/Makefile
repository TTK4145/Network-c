# program executable name
TARGET = test-tcp

# compiler flags
CFLAGS = -g -Wall

# linker flags
LDFLAGS = -g 

# list of sources
SOURCES = test-tcp.c ../tcp/tcp.c
INC= -I../udp

# default rule, to compile everything
all: $(TARGET)

# define object files
OBJECTS = $(SOURCES:.c=.o)

# link programs
$(TARGET): $(OBJECTS)
	gcc -o $@ $^ $(LDFLAGS) $(INC)
	mv $(TARGET) ./build/

# compile
%.o : %.c
	gcc -c $(CFLAGS) $(INC) -o $@ $<

# cleaning
clean:
	rm -fr ./build/* $(OBJECTS)
