
CC=g++
CFLAGS=-c -std=c++0x -O2
CVLIBS=-lopencv_core  -lopencv_highgui -lz

all: hashing 


hashing: main.o header.o
	$(CC)  main.o header.o -o hashing -fopenmp $(CVLIBS)

main.o: main.cpp header.h
	$(CC) $(CFLAGS) main.cpp -o main.o -fopenmp


header.o: header.cpp header.h 
	$(CC) $(CFLAGS) header.cpp


clean:
	rm -rf *o hashing get_precomp_feats


