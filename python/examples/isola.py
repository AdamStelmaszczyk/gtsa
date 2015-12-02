import itertools

from gtsa import State, Minimax, Tester, MonteCarloTreeSearch


SIDE = 3
PLAYER_1 = '1'
PLAYER_2 = '2'
EMPTY = '_'
REMOVED = '#'


def get_opposite_player(player):
    return PLAYER_2 if player == PLAYER_1 else PLAYER_1


def get_score_for_cords(x, y):
    return min(min(x + 1, SIDE - x), min(y + 1, SIDE - y))


class IsolaState(State):
    def __init__(self, init_string=None):
        super(IsolaState, self).__init__(PLAYER_1)

        self.board = [[EMPTY for _ in range(SIDE)] for _ in range(SIDE)]
        if init_string:
            correct_length = SIDE ** 2
            if len(init_string) != correct_length:
                raise ValueError("Initialization string length must be {}"
                                 .format(correct_length))
            for i, char in enumerate(init_string):
                if char not in [PLAYER_1, PLAYER_2, EMPTY, REMOVED]:
                    raise ValueError("Undefined symbol used: '{}'".
                                     format(char))
                x = i % SIDE
                y = i // SIDE
                self.board[y][x] = char

            self.player_1_cords = self.find_player_cords(PLAYER_1)
            self.player_2_cords = self.find_player_cords(PLAYER_2)

    def clone(self):
        clone = IsolaState()
        clone.board = [row[:] for row in self.board]
        clone.player_1_cords = self.player_1_cords
        clone.player_2_cords = self.player_2_cords
        return clone

    def get_goodness(self):
        x, y = self.get_player_cords(self.player_to_move)
        player_score = self.get_score_for_legal_steps(x, y, 2)
        if player_score == 0:
            return -1000
        enemy = get_opposite_player(self.player_to_move)
        x, y = self.get_player_cords(enemy)
        enemy_score = self.get_score_for_legal_steps(x, y, 2)
        if enemy_score == 0:
            return 1000
        return player_score - enemy_score

    def get_legal_moves(self):
        x, y = self.get_player_cords(self.player_to_move)
        step_moves = self.get_legal_step_moves(x, y)
        remove_moves = self.get_legal_remove_moves(self.player_to_move)
        product = itertools.product(step_moves, remove_moves)
        for (step_move, remove_move) in product:
            if step_move != remove_move:
                yield (x, y,
                       step_move[0], step_move[1],
                       remove_move[0], remove_move[1])

    def is_terminal(self):
        x, y = self.get_player_cords(self.player_to_move)
        return self.get_score_for_legal_steps(x, y) == 0

    def is_winner(self, player):
        enemy = get_opposite_player(player)
        x, y = self.get_player_cords(enemy)
        return self.player_to_move == enemy and \
            self.get_score_for_legal_steps(x, y) == 0

    def make_move(self, move):
        self.board[move[1]][move[0]] = EMPTY
        self.board[move[3]][move[2]] = self.player_to_move
        self.board[move[5]][move[4]] = REMOVED
        self.set_player_cords(self.player_to_move, (move[2], move[3]))
        self.player_to_move = get_opposite_player(self.player_to_move)

    def undo_move(self, move):
        self.player_to_move = get_opposite_player(self.player_to_move)
        self.set_player_cords(self.player_to_move, (move[0], move[1]))
        self.board[move[5]][move[4]] = EMPTY
        self.board[move[3]][move[2]] = EMPTY
        self.board[move[1]][move[0]] = self.player_to_move

    def __repr__(self):
        return '\n'.join([''.join(row) for row in self.board]) + '\n' + \
            self.player_to_move + '\n'

    def __hash__(self):
        return hash(tuple(tuple(row) for row in self.board)) + \
            hash(self.player_to_move)

    def __eq__(self, other):
        return self.board == other.board and \
            self.player_to_move == other.player_to_move

    def get_legal_remove_moves(self, player):
        # Prioritize remove moves around the enemy
        no_moves_around_enemy = True
        start_x, start_y = self.get_player_cords(get_opposite_player(player))
        for dy in range(-2, 3):
            for dx in range(-2, 3):
                x = start_x + dx
                y = start_y + dy
                if 0 <= x < SIDE and 0 <= y < SIDE and \
                        self.board[y][x] in [EMPTY, player]:
                    no_moves_around_enemy = False
                    yield (x, y)
        if no_moves_around_enemy:
            for y in range(SIDE):
                for x in range(SIDE):
                    if self.board[y][x] in [EMPTY, player]:
                        yield (x, y)

    def get_legal_step_moves(self, start_x, start_y):
        for dy in range(-1, 2):
            for dx in range(-1, 2):
                x = start_x + dx
                y = start_y + dy
                if 0 <= x < SIDE and 0 <= y < SIDE and \
                        self.board[y][x] == EMPTY:
                    yield (x, y)

    def get_score_for_legal_steps(self, start_x, start_y, depth=1):
        result = 0
        for dy in range(-1, 2):
            for dx in range(-1, 2):
                x = start_x + dx
                y = start_y + dy
                if 0 <= x < SIDE and 0 <= y < SIDE and \
                        self.board[y][x] == EMPTY:
                    if depth <= 1:
                        result += get_score_for_cords(x, y)
                    else:
                        result += self.get_score_for_legal_steps(
                            x,
                            y,
                            depth - 1,
                        )
        return result

    def get_player_cords(self, player):
        if player == PLAYER_1:
            return self.player_1_cords
        return self.player_2_cords

    def find_player_cords(self, player):
        for y in range(SIDE):
            for x in range(SIDE):
                if self.board[y][x] == player:
                    return x, y
        raise ValueError("No {} on the board".format(player))

    def set_player_cords(self, player, cords):
        if player == PLAYER_1:
            self.player_1_cords = cords
        else:
            self.player_2_cords = cords


def read_isola_move():
    message = "Enter space separated " \
              "from_x from_y step_x step_y remove_x remove_y: "
    # Both Python 2 and 3 will use raw_input() in place of input().
    try:
        input = raw_input
    except NameError:
        pass
    return tuple(map(int, input(message).split()))


if __name__ == "__main__":
    state = IsolaState("_2_"
                       "___"
                       "_1_")

    algorithm_1 = Minimax(PLAYER_1, PLAYER_2, max_seconds=1, verbose=True)
    algorithm_2 = MonteCarloTreeSearch(
        PLAYER_2,
        PLAYER_1,
        max_seconds=1,
        verbose=True,
    )

    tester = Tester(state, algorithm_1, algorithm_2)
    tester.start()
