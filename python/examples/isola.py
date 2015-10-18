import itertools

from gtsa import State, Minimax, MoveReader, Tester, MonteCarloTreeSearch


SIDE = 7
PLAYER_1 = '1'
PLAYER_2 = '2'
EMPTY = '_'
REMOVED = '#'


def get_opposite_player(player):
    return PLAYER_2 if player == PLAYER_1 else PLAYER_1


def get_score(options):
    if options == 0:
        return -50
    elif options == 1:
        return -10
    elif options == 2:
        return 0
    else:
        return options + 2


class IsolaState(State):
    def __init__(self, side, init_string=None):
        super(IsolaState, self).__init__()
        self.side = side

        self.board = [[EMPTY for _ in range(side)] for _ in range(side)]
        if init_string:
            correct_length = self.side ** 2
            if len(init_string) != correct_length:
                raise ValueError("Initialization string length must be {}"
                                 .format(correct_length))
            for i, char in enumerate(init_string):
                if char not in [PLAYER_1, PLAYER_2, EMPTY, REMOVED]:
                    raise ValueError("Undefined symbol used: '{}'".
                                     format(char))
                x = i % self.side
                y = i // self.side
                self.board[y][x] = char

            self.player_1_cords = self.find_player_cords(PLAYER_1)
            self.player_2_cords = self.find_player_cords(PLAYER_2)

    def clone(self):
        clone = IsolaState(self.side)
        clone.board = [row[:] for row in self.board]
        clone.player_1_cords = self.player_1_cords
        clone.player_2_cords = self.player_2_cords
        return clone

    def get_goodness(self, player):
        if self.is_winner(player):
            return 100
        enemy = get_opposite_player(player)
        if self.is_winner(enemy):
            return -100
        current_player_options = self.get_number_of_legal_steps(player)
        current_player_score = get_score(current_player_options)
        next_player_options = self.get_number_of_legal_steps(enemy)
        next_player_score = get_score(next_player_options)
        return current_player_score - next_player_score

    def get_legal_moves(self, player):
        x, y = self.get_player_cords(player)
        step_moves = self.get_legal_step_moves(x, y)
        remove_moves = self.get_legal_remove_moves(player)
        product = itertools.product(step_moves, remove_moves)
        for (step_move, remove_move) in product:
            if step_move != remove_move:
                yield (x, y,
                       step_move[0], step_move[1],
                       remove_move[0], remove_move[1])

    def is_terminal(self, player):
        return self.get_number_of_legal_steps(player) == 0

    def is_winner(self, player):
        return self.player_who_moved == player and \
            self.get_number_of_legal_steps(get_opposite_player(player)) == 0

    def make_move(self, move, player):
        x, y = self.get_player_cords(player)
        self.board[y][x] = EMPTY
        self.board[move[3]][move[2]] = player
        self.board[move[5]][move[4]] = REMOVED
        self.set_player_cords(player, (move[2], move[3]))
        self.player_who_moved = player

    def undo_move(self, move, player):
        self.board[move[5]][move[4]] = EMPTY
        self.board[move[1]][move[0]] = player
        self.board[move[3]][move[2]] = EMPTY
        self.set_player_cords(player, (move[0], move[1]))
        self.player_who_moved = get_opposite_player(player)

    def __repr__(self):
        return '\n'.join([''.join(row) for row in self.board]) + '\n'

    def get_legal_remove_moves(self, player):
        for y in range(self.side):
            for x in range(self.side):
                if self.board[y][x] in [EMPTY, player]:
                    yield (x, y)

    def get_legal_step_moves(self, start_x, start_y):
        for dy in range(-1, 2):
            for dx in range(-1, 2):
                x = start_x + dx
                y = start_y + dy
                if 0 <= x < self.side and 0 <= y < self.side and \
                        self.board[y][x] == EMPTY:
                    yield (x, y)

    def get_number_of_legal_steps(self, player):
        result = 0
        start_x, start_y = self.get_player_cords(player)
        for dy in range(-1, 2):
            for dx in range(-1, 2):
                x = start_x + dx
                y = start_y + dy
                if 0 <= x < self.side and 0 <= y < self.side and \
                        self.board[y][x] == EMPTY:
                    result += 1
        return result

    def get_player_cords(self, player):
        if player == PLAYER_1:
            return self.player_1_cords
        return self.player_2_cords

    def find_player_cords(self, player):
        for y in range(self.side):
            for x in range(self.side):
                if self.board[y][x] == player:
                    return x, y
        raise ValueError("No {} on the board:\n{}".format(player, self))

    def set_player_cords(self, player, cords):
        if player == PLAYER_1:
            self.player_1_cords = cords
        else:
            self.player_2_cords = cords


class IsolaMoveReader(MoveReader):
    def read(self):
        message = "Enter space separated step_x step_y remove_x remove_y: "
        x, y = state.get_player_cords(PLAYER_1)
        return (x, y) + tuple(map(int, input(message).split()))


if __name__ == "__main__":
    state = IsolaState(SIDE, "___2___"
                             "_______"
                             "_______"
                             "_______"
                             "_______"
                             "_______"
                             "___1___")

    algorithm_1 = Minimax(PLAYER_1, PLAYER_2, max_depth=1)
    algorithm_2 = MonteCarloTreeSearch(PLAYER_2,
                                       PLAYER_1,
                                       max_seconds=1,
                                       verbose=True)

    tester = Tester(state, algorithm_1, algorithm_2)
    tester.start()