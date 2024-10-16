CC=gcc
EDCFLAGS = -I include -Wall -DLINUX $(CFLAGS)
EDLDFLAGS = -ldcamapi $(LDFLAGS)