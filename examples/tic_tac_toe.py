from gtsa.gtsa import State, Move, MoveReader, Human, Tester, \
    Minimax


SIDE = 3
PLAYER_1 = 'X'
PLAYER_2 = 'O'
EMPTY = '_'


LINES = []
for y in range(SIDE):
    row = tuple((x, y) for x in range(SIDE))
    LINES.append(row)
for x in range(SIDE):
    col = tuple((x, y) for y in range(SIDE))
    LINES.append(col)
LINES.append(tuple((x, x) for x in range(SIDE)))
LINES.append(tuple((SIDE - x - 1, x) for x in range(SIDE)))


class TicTacToeState(State):
    def __init__(self, side, init_string=None):
        super(TicTacToeState, self).__init__()
        self.side = side

        self.board = [[EMPTY for _ in range(side)] for _ in range(side)]
        if init_string:
            correct_length = self.side ** 2
            if len(init_string) != correct_length:
                raise ValueError("Initialization string length must be {}".
                                 format(correct_length))
            for i, char in enumerate(init_string):
                if char not in [PLAYER_1, PLAYER_2, EMPTY]:
                    raise ValueError("Undefined symbol used: '{}'".
                                     format(char))
                x = i % self.side
                y = i // self.side
                self.board[y][x] = char

    def clone(self):
        clone = TicTacToeState(self.side)
        clone.board = [row[:] for row in self.board]
        return clone

    def get_goodness(self, current_player):
        goodness = 0
        counts = self.count_players_on_lines(current_player)
        for count in counts:
            if count[0] == 3:
                goodness += self.side ** 2
            elif count[1] == 3:
                goodness -= self.side ** 2
            elif count[0] == 2 and count[1] == 0:
                goodness += self.side
            elif count[1] == 2 and count[0] == 0:
                goodness -= self.side
            elif count[0] == 1 and count[1] == 0:
                goodness += 1
            elif count[1] == 1 and count[0] == 0:
                goodness -= 1
        return goodness

    def get_legal_moves(self, player):
        legal_moves = set()
        for y in range(self.side):
            for x in range(self.side):
                if self.board[y][x] == EMPTY:
                    legal_moves.add(TicTacToeMove(x, y))
        return legal_moves

    def get_opposite_player(self, player):
        return PLAYER_2 if player == PLAYER_1 else PLAYER_1

    def make_move(self, move, player):
        self.board[move.get_y()][move.get_x()] = player

    def undo_move(self, move, player):
        self.board[move.get_y()][move.get_x()] = EMPTY

    def is_terminal(self, player):
        legal_moves = self.get_legal_moves(player)
        if not legal_moves:
            return True
        counts = self.count_players_on_lines(player)
        for count in counts:
            if count[0] == 3 or count[1] == 3:
                return True
        return False

    def is_winner(self, player):
        counts = self.count_players_on_lines(player)
        for count in counts:
            if count[0] == 3:
                return True
        return False

    def count_players_on_lines(self, current_player):
        counts = []
        next_player = self.get_opposite_player(current_player)
        for line in LINES:
            player_places = 0
            enemy_places = 0
            for (x, y) in line:
                if self.board[y][x] == current_player:
                    player_places += 1
                elif self.board[y][x] == next_player:
                    enemy_places += 1
            counts.append((player_places, enemy_places))
        return counts

    def __repr__(self):
        return '\n'.join([''.join(row) for row in self.board]) + '\n'

    def __eq__(self, other):
        return self.board == other.board

    def __ne__(self, other):
        return not self.__eq__(other)

    def __hash__(self):
        return hash(tuple(tuple(row) for row in self.board))


class TicTacToeMove(Move):
    def __init__(self, x, y):
        self.x = x
        self.y = y

    def get_x(self):
        return self.x

    def get_y(self):
        return self.y

    def __repr__(self):
        return "{} {}".format(self.x, self.y)

    def __eq__(self, other):
        return self.x == other.x and self.y == other.y

    def __ne__(self, other):
        return not self.__eq__(other)

    def __hash__(self):
        return self.y * SIDE + self.x


class TicTacToeMoveReader(MoveReader):
    def read(self):
        message = "Enter space separated X and Y of your move: "
        x, y = map(int, input(message).split())
        return TicTacToeMove(x, y)


if __name__ == "__main__":
    state = TicTacToeState(SIDE, "___"
                                 "___"
                                 "___")

    algorithm_1 = Human(PLAYER_1, PLAYER_2, TicTacToeMoveReader(state))
    algorithm_2 = Minimax(PLAYER_2, PLAYER_1)

    tester = Tester(state, algorithm_1, algorithm_2)
    tester.start()
