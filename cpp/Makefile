CC=g++-4.8
FLAGS=-std=c++11 -O2

all: tic_tac_toe isola

tic_tac_toe:
	$(CC) $(FLAGS) tests/test_tic_tac_toe.cpp -o tests/test_tic_tac_toe.o

isola:
	$(CC) $(FLAGS) tests/test_isola.cpp -o tests/test_isola.o

clean:
	rm -f tests/*.o
