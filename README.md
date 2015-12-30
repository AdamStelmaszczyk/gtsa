# Game Tree Search Algorithms [![Build Status](https://travis-ci.org/AdamStelmaszczyk/gtsa.svg?branch=master)](https://travis-ci.org/AdamStelmaszczyk/gtsa)

Python and C++ libraries providing algorithms useful in AI bot programming.

Usage
---

1. Specify game rules by subclassing `State`. 
2. Create AI players with chosen algorithms. You can also play by yourself (`Human` class).
3. Read the board state and output the move (typical for programming contests). 
Alternatively, use `Tester` to play the whole game (typical for local testing).

Implemented algorithms
---

- [Minimax](https://en.wikipedia.org/wiki/Minimax) ([negamax](https://en.wikipedia.org/wiki/Negamax) variant) with [alpha-beta pruning](https://en.wikipedia.org/wiki/Alpha%E2%80%93beta_pruning), [iterative deepening]( https://chessprogramming.wikispaces.com/Iterative+Deepening), [transposition table](https://en.wikipedia.org/wiki/Transposition_table), [history heuristic](https://chessprogramming.wikispaces.com/History+Heuristic).
- [Monte Carlo tree search](https://en.wikipedia.org/wiki/Monte_Carlo_tree_search) with [UCT](
https://en.wikipedia.org/wiki/Monte_Carlo_tree_search#Exploration_and_exploitation).

Game examples
---

- Tic Tac Toe - [rules](https://github.com/AdamStelmaszczyk/gtsa/blob/master/games/tic_tac_toe.md), [Python code](https://github.com/AdamStelmaszczyk/gtsa/blob/master/python/examples/tic_tac_toe.py), [C++ code](https://github.com/AdamStelmaszczyk/gtsa/blob/master/cpp/examples/tic_tac_toe.cpp).
- Isola - [rules](https://github.com/AdamStelmaszczyk/gtsa/blob/master/games/isola.md), [Python code](https://github.com/AdamStelmaszczyk/gtsa/blob/master/python/examples/isola.py), [C++ code](https://github.com/AdamStelmaszczyk/gtsa/blob/master/cpp/examples/isola.cpp).

Priorities
---

1. Correctness in Python and C++.
2. Performance in C++.

Would like to help?
---
Suggestions are welcome on [Issues](https://github.com/AdamStelmaszczyk/gtsa/issues).
[Pull requests](https://github.com/AdamStelmaszczyk/gtsa/pulls) are also awesome.

I'm sending £20 to anybody who submits successful bug report.
