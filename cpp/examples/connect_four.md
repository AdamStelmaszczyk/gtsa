# Connect Four ([C++ implementation](https://github.com/AdamStelmaszczyk/gtsa/blob/master/cpp/examples/connect_four.cpp))

Also known as: Four in a Row, Four in a Line.

Connect Four is a two-player game in which players take turns dropping colored discs from the top into a grid. The discs fall straight down, occupying the next available space within the column.

<p align="center">
  <img src="https://upload.wikimedia.org/wikipedia/commons/a/ad/Connect_Four.gif"/>
</p>

The objective of the game is to connect four of one's own discs of the same color next to each other vertically, horizontally, or diagonally before your opponent. If there is no free space left, we have a draw.

Input
---
The input is an 8x7 matrix consisting only of `_`, `1` and `2`. Then another line follows with `1` or `2`, which is your player id.

Cells with `_` are empty. Cells with `1` contain player 1's disc. Cells with `2` contain player 2's disc.

In the given matrix, top-left is (0, 0) and bottom-right is (7, 6). The x-coordinate increases from top to bottom, and y-coordinate increases from left to right.

First input
---
```
________
________
________
________
________
________
________
1
```

Sample input
---
```
________
________
________
________
________
________
___1____
2
```

Output
---
Index of the column in which you want to throw your piece.

Sample output
---
```
3
```

Sample game
---

<img src="https://github.com/AdamStelmaszczyk/gtsa/blob/master/cpp/examples/connect_four.gif"/>

Challenges
---
- [HackerEarth - Battle Of Bots #3](https://www.hackerearth.com/battle-of-bots-3/multiplayer/fantastic-four/)
