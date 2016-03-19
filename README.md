# Game Tree Search Algorithms [![Build Status](https://travis-ci.org/AdamStelmaszczyk/gtsa.svg?branch=master)](https://travis-ci.org/AdamStelmaszczyk/gtsa)

C++ library for AI bot programming.

Usage
---

1. Specify game rules by subclassing `State` and `Move`. 
2. Create AI players with chosen algorithms. You can also play by yourself (`Human` class).
3. Read the state and output the move (typical for programming contests). 
Alternatively, use `Tester` to play couple of games (typical for local testing).

Implemented algorithms
---

- [Minimax](https://en.wikipedia.org/wiki/Minimax) ([negamax](https://en.wikipedia.org/wiki/Negamax) variant) with [alpha-beta pruning](https://en.wikipedia.org/wiki/Alpha%E2%80%93beta_pruning), [iterative deepening]( https://chessprogramming.wikispaces.com/Iterative+Deepening), [transposition table](https://en.wikipedia.org/wiki/Transposition_table), [history heuristic](https://chessprogramming.wikispaces.com/History+Heuristic).
- [Monte Carlo tree search](https://en.wikipedia.org/wiki/Monte_Carlo_tree_search) with [UCT](
https://en.wikipedia.org/wiki/Monte_Carlo_tree_search#Exploration_and_exploitation).

Game examples
---

- Tic Tac Toe - [game rules](https://github.com/AdamStelmaszczyk/gtsa/blob/master/games/tic_tac_toe.md), [code example](https://github.com/AdamStelmaszczyk/gtsa/blob/master/cpp/examples/tic_tac_toe.cpp).
- Isola - [game rules](https://github.com/AdamStelmaszczyk/gtsa/blob/master/games/isola.md), [code example](https://github.com/AdamStelmaszczyk/gtsa/blob/master/cpp/examples/isola.cpp).

Priorities
---

1. Correctness.
2. Compliance with environment typical for competitions. One file submission, compiled with one invocation of `g++`.
3. Performance.

Would like to help?
---
Suggestions welcome on [Issues](https://github.com/AdamStelmaszczyk/gtsa/issues).
[Pull requests](https://github.com/AdamStelmaszczyk/gtsa/pulls) are also awesome.

I'm transferring £50 to anybody who submits successful bug report.
