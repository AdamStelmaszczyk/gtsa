CC=g++-4.8
FLAGS=-std=c++11 -O2

all: tests/test_tic_tac_toe.o tests/test_isola.o tests/play_isola.o

test: all
	tests/test_tic_tac_toe.o
	tests/test_isola.o
	
valgrind: all
	valgrind --error-exitcode=1 --leak-check=full tests/test_tic_tac_toe.o
	valgrind --error-exitcode=1 --leak-check=full tests/test_isola.o
	
play_isola: tests/play_isola.o
	tests/play_isola.o

tests/test_tic_tac_toe.o: gtsa.hpp examples/tic_tac_toe.cpp tests/test_tic_tac_toe.cpp
	$(CC) $(FLAGS) tests/test_tic_tac_toe.cpp -o tests/test_tic_tac_toe.o

tests/test_isola.o: gtsa.hpp examples/isola.cpp tests/test_isola.cpp
	$(CC) $(FLAGS) tests/test_isola.cpp -o tests/test_isola.o

tests/play_isola.o: gtsa.hpp examples/isola.cpp tests/play_isola.cpp
	$(CC) $(FLAGS) tests/play_isola.cpp -o tests/play_isola.o

clean:
	rm -f tests/*.o
