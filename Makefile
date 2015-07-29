
CC=g++
CFLAGS=-c -std=c++0x -O2
CVLIBS=-lopencv_core  -lopencv_highgui -lz

all: hashing 


hashing: main_zlib.o header.o
	$(CC)  main_zlib.o header.o -o hashing -fopenmp $(CVLIBS)

main_zlib.o: main_zlib.cpp header.h
	$(CC) $(CFLAGS) main_zlib.cpp -o main.o -fopenmp


header.o: header.cpp header.h 
	$(CC) $(CFLAGS) header.cpp


clean:
	rm -rf *o hashing get_precomp_feats


