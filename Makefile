CC=gcc
CXX=g++
EDCFLAGS = -I include -Wall -DLINUX -O2 $(CFLAGS)
EDCXXFLAGS = -I include -Wall -DLINUX -O2 $(CXXFLAGS)
EDLDFLAGS = -ldcamapi $(LDFLAGS)
