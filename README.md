# Game Tree Search Algorithms (GTSA) [![Build Status](https://travis-ci.org/AdamStelmaszczyk/gtsa.svg?branch=master)](https://travis-ci.org/AdamStelmaszczyk/gtsa)

Python library providing algorithms useful in AI bot programming.

Usage
---

1. Specify game rules by subclassing `State` and `Move`. 
2. Create AI players with chosen algorithms. You can also play by yourself (`Human` class).
3. Read the board state and output the move (typical for programming contests). 
Alternatively, use `Tester` to play the whole game (typical for local testing).

Check game [examples](https://github.com/AdamStelmaszczyk/gtsa/blob/master/examples/README.md). 

Implemented algorithms:
---

- [Minimax](https://en.wikipedia.org/wiki/Minimax) with [transposition table](https://en.wikipedia.org/wiki/Transposition_table)

Planned:
---

- [Alpha-beta pruning](https://en.wikipedia.org/wiki/Alpha%E2%80%93beta_pruning)
- [Monte Carlo Tree Search](https://en.wikipedia.org/wiki/Monte_Carlo_tree_search)
- [Neural networks](https://en.wikipedia.org/wiki/Artificial_neural_network)
