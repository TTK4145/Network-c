# program executable name
TARGET = test-udp

# compiler flags
CFLAGS = -g -Wall

# linker flags
LDFLAGS = -g 

# list of sources
SOURCES = test-udp.c ../udp/udp.c
INC= -I../udp

# default rule, to compile everything
all: $(TARGET)

# define object files
OBJECTS = $(SOURCES:.c=.o)

# link programs
$(TARGET): $(OBJECTS)
	gcc -o $@ $^ $(LDFLAGS) $(INC)
	cp $(TARGET) ./build/

# compile
%.o : %.c
	gcc -c $(CFLAGS) $(INC) -o $@ $<

# cleaning
clean:
	rm -fr ./build/* $(OBJECTS)
