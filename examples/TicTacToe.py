from gtsa.gtsa import State, Move, MoveReader, Human, Minimax, Tester


SIDE = 3
PLAYER_1 = 'X'
PLAYER_2 = 'O'
EMPTY = '_'


class TicTacToeState(State):
    def __init__(self, side, string):
        super(TicTacToeState, self).__init__()
        self.side = side

        self.board = [[EMPTY for _ in range(side)] for _ in range(side)]
        correct_length = self.side ** 2
        if len(string) != correct_length:
            raise ValueError("Initialization string length must be {}".format(correct_length))
        for i, char in enumerate(string):
            if char not in [PLAYER_1, PLAYER_2, EMPTY]:
                raise ValueError("Undefined symbol used: {}".format(char))
            x = i % self.side
            y = i // self.side
            self.board[y][x] = char

        self.lines = []
        for y in range(side):
            row = tuple((x, y) for x in range(side))
            self.lines.append(row)
        for x in range(side):
            col = tuple((x, y) for y in range(side))
            self.lines.append(col)
        self.lines.append(tuple((x, x) for x in range(side)))
        self.lines.append(tuple((side - x - 1, x) for x in range(side)))

    def get_goodness(self, current_player, next_player):
        goodness = 0
        counts = self.count_players_on_lines(current_player, next_player)
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

    def make_move(self, move, player):
        self.board[move.get_y()][move.get_x()] = player

    def undo_move(self, move, player):
        self.board[move.get_y()][move.get_x()] = EMPTY

    def is_terminal(self, current_player, next_player):
        legal_moves = self.get_legal_moves(current_player)
        if not legal_moves:
            return True
        counts = self.count_players_on_lines(current_player, next_player)
        for count in counts:
            if count[0] == 3 or count[1] == 3:
                return True
        return False

    def count_players_on_lines(self, current_player, next_player):
        counts = []
        for line in self.lines:
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
        return '\n'.join(['|'.join(row) for row in self.board]) + '\n'

    def __eq__(self, other):
        return self.board == other.board

    def __ne__(self, other):
        return not self.__eq__(self, other)

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
        return not self.__eq__(self, other)

    def __hash__(self):
        return self.y * SIDE + self.x


class TicTacToeMoveReader(MoveReader):
    def read(self):
        x, y = map(int, input("Enter space separated X and Y of your move: ").split())
        return TicTacToeMove(x, y)


if __name__ == "__main__":
    state = TicTacToeState(SIDE, "___"
                                 "___"
                                 "___")

    algorithm_1 = Human(PLAYER_1, PLAYER_2, TicTacToeMoveReader(state))
    algorithm_2 = Minimax(PLAYER_2, PLAYER_1, 10)

    tester = Tester(state, algorithm_1, algorithm_2)
    tester.start()