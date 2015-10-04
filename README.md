# Game Tree Search Algorithms (GTSA) [![Build Status](https://travis-ci.org/AdamStelmaszczyk/gtsa.svg?branch=master)](https://travis-ci.org/AdamStelmaszczyk/gtsa)

Python library providing algorithms useful in AI bot programming.

Usage
---

1. Specify game rules by subclassing `State` and `Move`. 
2. Create AI players with chosen algorithms. You can also play by yourself (`Human` class).
3. Read the board state and output the move (typical for programming contests). 
Alternatively, use `Tester` to play the whole game (typical for local testing).

Game examples
---

- Tic Tac Toe - [rules](https://github.com/AdamStelmaszczyk/gtsa/blob/master/examples/tic_tac_toe.md), [code](https://github.com/AdamStelmaszczyk/gtsa/blob/master/examples/tic_tac_toe.py)
- Isola - [rules](https://github.com/AdamStelmaszczyk/gtsa/blob/master/examples/isola.md), [code](https://github.com/AdamStelmaszczyk/gtsa/blob/master/examples/isola.py)

Planned:

- [Hex](https://www.hackerrank.com/challenges/hex)
- [Checkers](https://www.hackerrank.com/challenges/checkers)

Implemented algorithms:
---

- [Minimax](https://en.wikipedia.org/wiki/Minimax) with [alpha-beta pruning](https://en.wikipedia.org/wiki/Alpha%E2%80%93beta_pruning)
- [Monte Carlo tree search](https://en.wikipedia.org/wiki/Monte_Carlo_tree_search) with [UCT](
https://en.wikipedia.org/wiki/Monte_Carlo_tree_search#Exploration_and_exploitation)

Planned:

- [Neural networks](https://en.wikipedia.org/wiki/Artificial_neural_network)
