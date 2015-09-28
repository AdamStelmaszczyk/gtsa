# [Isola](https://github.com/AdamStelmaszczyk/gtsa/blob/master/examples/Isola.py)

Also known as: Isolation, Stranded.

Isola is a two-player board game. It is played on a 7x7 grid which is initially filled with squares. Both players have one piece; it is in the middle position of the row closest to his/her side of the board. Players can place their piece on empty squares only. 

<p align="center">
  <img src="https://upload.wikimedia.org/wikipedia/commons/8/8a/Isola_starting_position.png"/>
</p>

A move consists of two subsequent actions:

1. Moving a piece to a neighboring (horizontally, vertically, or diagonally) position that contains an empty square.
2. Removing an empty square.

The player who cannot make any move loses the game.

Input 
---
The input is an 7x7 matrix consisting only of `_`, `1`, `2` and `#`. Then another line follows with `1` or `2`, which is your player id.

In the given matrix, top-left is (0, 0) and bottom-right is (6, 6). The x-coordinate increases from top to bottom, and y-coordinate increases from left to right.

The cell marked `_` means it contains an empty square. The cell marked `1` means it contains player 1's piece. The cell marked `2` means it contains player 2's piece. The cell marked `#` means it doesn't contain the square.

Starting state
---
```
___2___
_______
_______
_______
_______
_______
___1___
```

Sample input
---
```
___2___
_______
___#___
_______
_______
___1___
_______
2
```

Output 
---
The coordinates of the empty square (x, y) where you want to move your piece. 
In next line, the (x, y) coordinates of empty square to remove.

Sample output
---
```
3 5
3 2
```

Challenges
---
- [HackerEarth - Battle of Bots 2015](https://www.hackerearth.com/problem/multiplayer/isola/)
