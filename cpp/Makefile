CC=g++
FLAGS=-std=c++11 -O2 -fprofile-arcs -ftest-coverage -Wreturn-type -D BOOST_MATH_NO_LONG_DOUBLE_MATH_FUNCTIONS

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

test_executable: tests/test_executable.o tests/marten.o
	tests/test_executable.o

marten: tests/marten.o
	tests/marten.o

play_isola: tests/play_isola.o
	tests/play_isola.o

tests/test_tic_tac_toe.o: gtsa.hpp examples/tic_tac_toe.cpp tests/test_tic_tac_toe.cpp
	$(CC) $(FLAGS) tests/test_tic_tac_toe.cpp -o tests/test_tic_tac_toe.o

tests/test_isola.o: gtsa.hpp examples/isola.cpp tests/test_isola.cpp
	$(CC) $(FLAGS) tests/test_isola.cpp -o tests/test_isola.o

tests/test_connect_four.o: gtsa.hpp examples/connect_four.cpp tests/test_connect_four.cpp
	$(CC) $(FLAGS) tests/test_connect_four.cpp -o tests/test_connect_four.o

tests/test_executable.o: gtsa.hpp examples/isola.cpp tests/test_executable.cpp
	$(CC) $(FLAGS) tests/test_executable.cpp -o tests/test_executable.o

tests/marten.o: tests/marten.cpp
	$(CC) $(FLAGS) tests/marten.cpp -o tests/marten.o

tests/play_isola.o: gtsa.hpp examples/isola.cpp tests/play_isola.cpp
	$(CC) $(FLAGS) tests/play_isola.cpp -o tests/play_isola.o

clean:
	rm -f tests/*.o *.gcov *.gcda *.gcno
