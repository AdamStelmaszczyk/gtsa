# [Tic Tac Toe](https://github.com/AdamStelmaszczyk/gtsa/blob/master/examples/TicTacToe.py)

Also known as: Noughts and Crosses, Xs and Os.

Tic Tac Toe is a pencil-and-paper game for two players, X and O, who take turns marking the spaces in a 3x3 grid. The player who succeeds in placing three respective marks in a horizontal, vertical, or diagonal row wins the game.

<p align="center">
  <img src="https://upload.wikimedia.org/wikipedia/commons/thumb/1/1b/Tic-tac-toe-game-1.svg/479px-Tic-tac-toe-game-1.svg.png"/>
</p>

Input 
---
The input is 3x3 matrix consisting only of `X`, `O` or `_`. Then another line follows with either `X` or `O`, denoting your symbol.

`_` denotes empty cell. `X` denotes cell belonging to X player. `O` denotes cell belonging to O player.

In the given matrix, top-left is (0, 0) and bottom-right is (2, 2). The x-coordinate increases from top to bottom, and y-coordinate increases from left to right. 

Starting state
---
```
___
___
___
```

Sample input
---
```
OX_
_X_
___
O
```

Output 
---
The coordinates of the cell (x, y) where you want to move. 

Sample output
---
`1 1`

Challenges
---
- [HackerRank](https://www.hackerrank.com/challenges/tic-tac-toe)
- [HackerEarth - India Hacks 2013 Teaser](https://www.hackerearth.com/problem/multiplayer/tic-tac-toe) 
