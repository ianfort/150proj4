FATTest : FATData.o main.o
	g++ -ansi -Wall -o FATTest -g   FATData.o main.o

FATData.o : FATData.cpp FATData.h
	g++ -ansi -Wall -g -c  FATData.cpp

main.o : main.cpp FATData.h
	g++ -ansi -Wall -g -c  main.cpp

clean :
	rm -f FATTest FATData.o main.o