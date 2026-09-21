GXX=gcc
GXXFLAG=-g -Wall -std=c++20

main: main.o
	$(GXX) $(GXXFLAG) $^ -o main

main.o: miniefsm.hpp
	$(GXX) $(GXXFLAG) -c main.cpp miniefsm.hpp 

clean:
	rm -f *.o *.gch main

