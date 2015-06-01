FATTest : Dir.o FATData.o main.o
	g++ -ansi -Wall -o FATTest -g   Dir.o FATData.o main.o

Dir.o : Dir.cpp Dir.h
	g++ -ansi -Wall -g -c  Dir.cpp

FATData.o : FATData.cpp FATData.h
	g++ -ansi -Wall -g -c  FATData.cpp

main.o : main.cpp FATData.h
	g++ -ansi -Wall -g -c  main.cpp

clean :
	rm -f FATTest Dir.o FATData.o main.o