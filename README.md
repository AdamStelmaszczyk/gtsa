# Game Tree Search Algorithms [![Build Status](https://app.travis-ci.com/AdamStelmaszczyk/gtsa.svg?branch=master)](https://app.travis-ci.com/AdamStelmaszczyk/gtsa) [![Coverage Status](https://coveralls.io/repos/github/AdamStelmaszczyk/gtsa/badge.svg?branch=master)](https://coveralls.io/github/AdamStelmaszczyk/gtsa?branch=master)

C++ library for AI bot programming.

Usage
---

1. Specify game rules by subclassing `State` and `Move`. 
2. Choose AI algorithm (`Minimax`, `MonteCarloTreeSearch`), play by yourself (`Human`) or against external program (`Executable`).
3. Read the game state, pass it to the algorithm and output the move. For local, statistically significant tests, use `Tester`.

Game examples
---

- Tic Tac Toe - [game rules](https://github.com/AdamStelmaszczyk/gtsa/blob/master/cpp/examples/tic_tac_toe.md), [code example](https://github.com/AdamStelmaszczyk/gtsa/blob/master/cpp/examples/tic_tac_toe.cpp).
<p><a href="https://github.com/AdamStelmaszczyk/gtsa/blob/master/cpp/examples/tic_tac_toe.md"><img src="https://github.com/AdamStelmaszczyk/gtsa/blob/master/cpp/examples/tic_tac_toe.gif"/></a></p>

- Isola - [game rules](https://github.com/AdamStelmaszczyk/gtsa/blob/master/cpp/examples/isola.md), [code example](https://github.com/AdamStelmaszczyk/gtsa/blob/master/cpp/examples/isola.cpp). 
<p><a href="https://github.com/AdamStelmaszczyk/gtsa/blob/master/cpp/examples/isola.md"><img src="https://github.com/AdamStelmaszczyk/gtsa/blob/master/cpp/examples/isola.gif"/></a></p>

- Connect Four - [game rules](https://github.com/AdamStelmaszczyk/gtsa/blob/master/cpp/examples/connect_four.md), [code example](https://github.com/AdamStelmaszczyk/gtsa/blob/master/cpp/examples/connect_four.cpp).
<p><a href="https://github.com/AdamStelmaszczyk/gtsa/blob/master/cpp/examples/connect_four.md"><img src="https://github.com/AdamStelmaszczyk/gtsa/blob/master/cpp/examples/connect_four.gif"/></a></p>

- Go - [game rules](https://github.com/AdamStelmaszczyk/gtsa/blob/master/cpp/examples/go.md), [code example](https://github.com/AdamStelmaszczyk/gtsa/blob/master/cpp/examples/go.cpp).
<p><a href="https://github.com/AdamStelmaszczyk/gtsa/blob/master/cpp/examples/go.md"><img src="https://github.com/AdamStelmaszczyk/gtsa/blob/master/cpp/examples/go.gif"/></a></p>

Implemented algorithms
---

- [NegaScout](https://en.wikipedia.org/wiki/Principal_variation_search) with [iterative deepening]( https://chessprogramming.wikispaces.com/Iterative+Deepening) and [transposition table](https://en.wikipedia.org/wiki/Transposition_table).
- [Monte Carlo tree search](https://en.wikipedia.org/wiki/Monte_Carlo_tree_search) with [UCT](
https://en.wikipedia.org/wiki/Monte_Carlo_tree_search#Exploration_and_exploitation) and [virtual visits](https://github.com/AdamStelmaszczyk/gtsa/issues/18).

Both handle sequential, multiplayer games:

<img src="https://github.com/AdamStelmaszczyk/gtsa/blob/master/cpp/examples/isola_four.gif"/></a></p>

MCTS also handles simultaneous games using [SUCT](http://mlanctot.info/files/papers/cig14-smmctsggp.pdf).

Make commands
---
Execute below commands in the `cpp` directory. 
- `make` builds everything.
- `make test` runs unit tests.
- `make valgrind` runs valgrind's memory leak tests.
- `make play_isola` plays as many games as needed to determine which Isola bot is better.

For all the commands check [`Makefile` file](https://github.com/AdamStelmaszczyk/gtsa/blob/master/cpp/Makefile).

Dependencies
---
- To build and run unit tests, `g++` and `libboost-all-dev` packages are needed.  
- To run valgrind, `valgrind` package is needed.  
- To make GIFs, `imagemagick` package is needed.

You can install all of them with `sudo apt-get install g++ libboost-all-dev valgrind imagemagick`.

Priorities of the library
---

1. Correctness.
2. Compliance with environment typical for competitions. One process. One file submission, compiled with one invocation of `g++`.
3. Performance.

Would like to help?
---
Suggestions welcome on [Issues](https://github.com/AdamStelmaszczyk/gtsa/issues).
[Pull requests](https://github.com/AdamStelmaszczyk/gtsa/pulls) too.

If you use code or ideas from this repository, please cite it as follows:

```
@misc{stelmaszczyk2015gtsa,
    author = {Stelmaszczyk, Adam},
    title = {Game Tree Search Algorithms},
    year = {2015},
    publisher = {GitHub},
    journal = {GitHub repository},
    howpublished = {\url{https://github.com/AdamStelmaszczyk/gtsa}},
}
```
