
CC=g++
CFLAGS=-c -std=c++0x -O2
CVLIBS=-lopencv_core  -lopencv_highgui

all: hashing get_precomp_feats

get_precomp_feats: get_precomp_feats.o header.o
	$(CC)  get_precomp_feats.o header.o -o get_precomp_feats -fopenmp $(CVLIBS)

hashing: main.o header.o
	$(CC)  main.o header.o -o hashing -fopenmp $(CVLIBS)

main.o: main.cpp header.h
	$(CC) $(CFLAGS) main.cpp -o main.o -fopenmp

main.o: get_precomp_feats.cpp header.h
	$(CC) $(CFLAGS) get_precomp_feats.cpp -o get_precomp_feats.o -fopenmp

header.o: header.cpp header.h 
	$(CC) $(CFLAGS) header.cpp


clean:
	rm -rf *o hashing


