CC=gcc
CXX=g++
EDCFLAGS = -I include -Wall -DLINUX -O2 $(CFLAGS)
EDCXXFLAGS = -I include -Wall -DLINUX -O2 $(CXXFLAGS)
EDLDFLAGS = -ldcamapi $(LDFLAGS)

PNG_CFLAGS = $(shell libpng-config --cflags)
PNG_LDFLAGS = $(shell libpng-config --ldflags)

SRCS = $(wildcard src/*.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))

LIBTARGET = lib/liborcaapi.a
EXAMPLES = $(patsubst %.cpp,%.exe,$(wildcard examples/*.cpp))

all: $(LIBTARGET) $(EXAMPLES)

$(LIBTARGET): $(OBJS)
	@mkdir -p lib
	ar rcs $@ $^

%.o: %.c
	$(CC) -c -o $@ $< $(EDCFLAGS)

%.exe: %.cpp $(LIBTARGET)
	$(CXX) -o $@ $< $(LIBTARGET) $(EDCXXFLAGS) $(PNG_CFLAGS) $(EDLDFLAGS) $(PNG_LDFLAGS)

clean:
	rm -vf $(OBJS) $(LIBTARGET) $(EXAMPLES)