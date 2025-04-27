# Go ([C++ implementation](https://github.com/AdamStelmaszczyk/gtsa/blob/master/cpp/examples/go.cpp))

Also known as: Weiqi, Baduk.

Go is a two-player board game. [Tromp-Taylor](http://tromp.github.io/go.html) rules (similar to Chinese):

1. Go is played on a NxN square grid of points, by two players called Black and White.
2. Each point on the grid may be colored black, white or empty.
3. A point P, not colored C, is said to reach C, if there is a path of (vertically or horizontally)
adjacent points of P's color from P to a point of color C.
4. Clearing a color is the process of emptying all points of that color that don't reach empty.
5. Starting with an empty grid, the players alternate turns, starting with Black.
6. A turn is either a pass; or a move that doesn't repeat an earlier grid coloring.
7. A move consists of coloring an empty point one's own color;
then clearing the opponent color, and then clearing one's own color.
8. The game ends after two consecutive passes.
9. A player's score is the number of points of her color, plus the number of empty points that reach only her color.
10. The player with the higher score at the end of the game is the winner. Equal scores result in a tie.

<p align="center">
  <img src="https://upload.wikimedia.org/wikipedia/commons/thumb/9/9f/Fineart_vs_Golaxy.gif/220px-Fineart_vs_Golaxy.gif"/>
</p>

Input
---
The input is a 5x5 matrix consisting only of `1`, `2` and `_`. Then another line follows with `1` or `2`, which is your player id.

The cell marked `_` means it contains an empty square. The cell marked `1` means it contains player 1's point. The cell marked `2` means it contains player 2's point.

In the given matrix, top-left is (0, 0) and bottom-right is (4, 4). The x-coordinate increases from top to bottom, and y-coordinate increases from left to right.

First input
---
```
_____
_____
_____
_____
_____
1
```

Sample input
---
```
_____
_22__
_1122
___11
_____
2
```

Output
---
The coordinates of the empty square (x, y) where you want to move.

Sample output
---
```
2 4
```

Sample game
---
<img src="https://github.com/AdamStelmaszczyk/gtsa/blob/master/cpp/examples/go.gif"/>
