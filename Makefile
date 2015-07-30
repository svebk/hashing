INCLUDE_DIRS=/opt/local/include/
LIB_DIRS=/opt/local/lib/
CC=g++
CFLAGS=-c -std=c++0x -O2
CVLIBS=-lopencv_core -lopencv_highgui -lz

all: hashing compress_feats

hashing: main.o header.o
	$(CC)  main.o header.o -o hashing -fopenmp $(CVLIBS) -L$(LIB_DIRS)

compress_feats: compress_feats.o header.o
	$(CC)  compress_feats.o header.o -o compress_feats -I$(INCLUDE_DIRS) $(CVLIBS) -L$(LIB_DIRS)

main.o: main.cpp header.h
	$(CC) $(CFLAGS) main.cpp -o main.o -fopenmp -I$(INCLUDE_DIRS)

compress_feats.o: compress_feats.cpp header.h
	$(CC) $(CFLAGS) compress_feats.cpp -o compress_feats.o -I$(INCLUDE_DIRS)

header.o: header.cpp header.h 
	$(CC) $(CFLAGS) header.cpp


clean:
	rm -rf *o hashing get_precomp_feats


