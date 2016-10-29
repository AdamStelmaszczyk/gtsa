CC=g++-4.8
FLAGS=-std=c++11 -O2 -fprofile-arcs -ftest-coverage -Wreturn-type

all: tests/test_tic_tac_toe.o tests/test_isola.o tests/test_connect_four.o tests/play_isola.o

test: tests/test_tic_tac_toe.o tests/test_isola.o tests/test_connect_four.o
	tests/test_tic_tac_toe.o
	tests/test_isola.o
	tests/test_connect_four.o
	
valgrind: tests/test_tic_tac_toe.o tests/test_isola.o tests/test_connect_four.o
	valgrind --error-exitcode=1 --leak-check=full tests/test_tic_tac_toe.o
	valgrind --error-exitcode=1 --leak-check=full tests/test_isola.o
	valgrind --error-exitcode=1 --leak-check=full tests/test_connect_four.o

test_tic_tac_toe: tests/test_tic_tac_toe.o
	tests/test_tic_tac_toe.o

test_isola: tests/test_isola.o
	tests/test_isola.o

test_connect_four: tests/test_connect_four.o
	tests/test_connect_four.o
	
play_isola: tests/play_isola.o
	tests/play_isola.o

tests/test_tic_tac_toe.o: gtsa.hpp examples/tic_tac_toe.cpp tests/test_tic_tac_toe.cpp
	$(CC) $(FLAGS) tests/test_tic_tac_toe.cpp -o tests/test_tic_tac_toe.o

tests/test_isola.o: gtsa.hpp examples/isola.cpp tests/test_isola.cpp
	$(CC) $(FLAGS) tests/test_isola.cpp -o tests/test_isola.o
	
tests/test_connect_four.o: gtsa.hpp examples/connect_four.cpp tests/test_connect_four.cpp
	$(CC) $(FLAGS) tests/test_connect_four.cpp -o tests/test_connect_four.o

tests/play_isola.o: gtsa.hpp examples/isola.cpp tests/play_isola.cpp
	$(CC) $(FLAGS) tests/play_isola.cpp -o tests/play_isola.o

gif: $(foreach dir, ., $(wildcard *.txt))
	for f in $^; do cat $$f | text2gif > `echo $$f | sed 's/txt/gif/'`; done
	convert -delay 100 `ls *.gif | sort -g` -loop 0 game.gif
	rm [0-9]*.gif

clean:
	rm -f tests/*.o
