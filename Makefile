GXX=g++
GXXFLAG=-g -Wall -std=c++20 -fconcepts-diagnostics-depth=3
HEADERS=miniefsm.hpp typelist.hpp state.hpp transition.hpp policy.hpp

main: main.o
	$(GXX) $(GXXFLAG) $^ -o main

main.o: $(HEADERS) main.cpp
	$(GXX) $(GXXFLAG) -c main.cpp 

clean:
	rm -f *.o *.gch main

