from gtsa import State, Human, Tester, Minimax


SIDE = 3
PLAYER_1 = 'X'
PLAYER_2 = 'O'
EMPTY = '_'


def get_opposite_player(player):
    return PLAYER_2 if player == PLAYER_1 else PLAYER_1


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
    def __init__(self, init_string=None):
        super(TicTacToeState, self).__init__(PLAYER_1)

        self.board = [[EMPTY for _ in range(SIDE)] for _ in range(SIDE)]
        if init_string:
            correct_length = SIDE ** 2
            if len(init_string) != correct_length:
                raise ValueError("Initialization string length must be {}".
                                 format(correct_length))
            for i, char in enumerate(init_string):
                if char not in [PLAYER_1, PLAYER_2, EMPTY]:
                    raise ValueError("Undefined symbol used: '{}'".
                                     format(char))
                x = i % SIDE
                y = i // SIDE
                self.board[y][x] = char

    def clone(self):
        clone = TicTacToeState()
        clone.board = [row[:] for row in self.board]
        return clone

    def get_goodness(self):
        goodness = 0
        counts = self.count_players_on_lines(self.player_to_move)
        for count in counts:
            if count[0] == SIDE:
                goodness += SIDE ** 2
            elif count[1] == SIDE:
                goodness -= SIDE ** 2
            elif count[0] == SIDE - 1 and count[1] == 0:
                goodness += SIDE
            elif count[1] == SIDE - 1 and count[0] == 0:
                goodness -= SIDE
            elif count[0] == SIDE - 2 and count[1] == 0:
                goodness += 1
            elif count[1] == SIDE - 2 and count[0] == 0:
                goodness -= 1
        return goodness

    def get_legal_moves(self):
        for y in range(SIDE):
            for x in range(SIDE):
                if self.board[y][x] == EMPTY:
                    yield (x, y)

    def is_terminal(self):
        if not self.has_empty_space():
            return True
        for count in self.count_players_on_lines(self.player_to_move):
            if count[0] == SIDE or count[1] == SIDE:
                return True
        return False

    def is_winner(self, player):
        for count in self.count_players_on_lines(player):
            if count[0] == SIDE:
                return True
        return False

    def make_move(self, move):
        self.board[move[1]][move[0]] = self.player_to_move
        self.player_to_move = get_opposite_player(self.player_to_move)

    def undo_move(self, move):
        self.board[move[1]][move[0]] = EMPTY
        self.player_to_move = get_opposite_player(self.player_to_move)

    def __repr__(self):
        return '\n'.join([''.join(row) for row in self.board]) + '\n'

    def __hash__(self):
        return hash(tuple(tuple(row) for row in self.board))

    def __eq__(self, other):
        return self.board == other.board

    def count_players_on_lines(self, current_player):
        next_player = get_opposite_player(current_player)
        for line in LINES:
            player_places = 0
            enemy_places = 0
            for (x, y) in line:
                if self.board[y][x] == current_player:
                    player_places += 1
                elif self.board[y][x] == next_player:
                    enemy_places += 1
            yield (player_places, enemy_places)

    def has_empty_space(self):
        for y in range(SIDE):
            for x in range(SIDE):
                if self.board[y][x] == EMPTY:
                    return True
        return False


def read_tic_tac_toe_move():
    message = "Enter space separated X and Y of your move: "
    # Both Python 2 and 3 will use raw_input() in place of input().
    try:
        input = raw_input
    except NameError:
        pass
    return tuple(map(int, input(message).split()))


if __name__ == "__main__":
    state = TicTacToeState("___"
                           "___"
                           "___")

    algorithm_1 = Minimax(PLAYER_1, PLAYER_2, verbose=True)
    algorithm_2 = Human(PLAYER_2, PLAYER_1, read_tic_tac_toe_move)

    tester = Tester(state, algorithm_1, algorithm_2)
    tester.start()
