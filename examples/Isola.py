from gtsa.gtsa import State, Move, Minimax, MoveReader, Tester


SIDE = 5
PLAYER_1 = '1'
PLAYER_2 = '2'
EMPTY = '_'
REMOVED = '#'


class IsolaState(State):
    def __init__(self, side, string):
        super(IsolaState, self).__init__()
        self.side = side

        self.board = [[' ' for _ in range(side)] for _ in range(side)]
        correct_length = self.side ** 2
        if len(string) != correct_length:
            raise ValueError("Initialization string length must be {}".format(correct_length))
        for i, char in enumerate(string):
            x = i % self.side
            y = i // self.side
            self.board[y][x] = char

        self.player_1_cords = self.find_player_cords(PLAYER_1)
        self.player_2_cords = self.find_player_cords(PLAYER_2)

    def get_goodness(self, current_player, next_player):
        current_player_freedom = self.get_number_of_legal_moves(current_player)
        next_player_freedom = self.get_number_of_legal_moves(next_player)
        return current_player_freedom - next_player_freedom

    def get_number_of_legal_moves(self, player):
        x, y = self.get_player_cords(player)
        return len(self.get_legal_step_moves(x, y))

    def get_legal_moves(self, player):
        x, y = self.get_player_cords(player)
        step_moves = self.get_legal_step_moves(x, y)
        remove_moves = self.get_legal_remove_moves(player)
        legal_moves = set()
        for step_move in step_moves:
            for remove_move in remove_moves:
                if step_move != remove_move:
                    legal_moves.add(IsolaMove(x, y, step_move[0], step_move[1], remove_move[0], remove_move[1]))
        return legal_moves

    def get_legal_step_moves(self, start_x, start_y):
        step_moves = set()
        for dy in range(-1, 2):
            for dx in range(-1, 2):
                x = start_x + dx
                y = start_y + dy
                if x < 0 or x >= self.side or y < 0 or y >= self.side:
                    continue
                if self.board[y][x] == EMPTY:
                    step_moves.add((x, y))
        return step_moves

    def get_legal_remove_moves(self, player):
        remove_moves = set()
        for y in range(self.side):
            for x in range(self.side):
                if self.board[y][x] in [EMPTY, player]:
                    remove_moves.add((x, y))
        return remove_moves

    def make_move(self, move, player):
        x, y = self.get_player_cords(player)
        self.board[y][x] = EMPTY
        self.board[move.get_step_y()][move.get_step_x()] = player
        self.board[move.get_remove_y()][move.get_remove_x()] = REMOVED
        self.set_player_cords(player, (move.get_step_x(), move.get_step_y()))

    def undo_move(self, move, player):
        self.board[move.get_remove_y()][move.get_remove_x()] = EMPTY
        self.board[move.get_from_y()][move.get_from_x()] = player
        self.board[move.get_step_y()][move.get_step_x()] = EMPTY
        self.set_player_cords(player, (move.get_from_x(), move.get_from_y()))

    def is_terminal(self, current_player, next_player):
        x, y = self.get_player_cords(current_player)
        current_player_legal_steps = self.get_legal_step_moves(x, y)
        x, y = self.get_player_cords(next_player)
        next_player_legal_steps = self.get_legal_step_moves(x, y)
        return not current_player_legal_steps or not next_player_legal_steps

    def find_player_cords(self, player):
        for y in range(self.side):
            for x in range(self.side):
                if self.board[y][x] == player:
                    return x, y
        raise ValueError("No {} on the board:\n{}".format(player, self))

    def get_player_cords(self, player):
        return self.player_1_cords if player == PLAYER_1 else self.player_2_cords

    def set_player_cords(self, player, cords):
        if player == PLAYER_1:
            self.player_1_cords = cords
        else:
            self.player_2_cords = cords

    def __repr__(self):
        return '\n'.join(['|'.join(row) for row in self.board]) + '\n'

    def __eq__(self, other):
        return self.board == other.board

    def __ne__(self, other):
        return not self.__eq__(self, other)

    def __hash__(self):
        return hash(tuple(tuple(row) for row in self.board))


class IsolaMove(Move):
    def __init__(self, from_x, from_y, step_x, step_y, remove_x, remove_y):
        self.from_x = from_x
        self.from_y = from_y
        self.step_x = step_x
        self.step_y = step_y
        self.remove_x = remove_x
        self.remove_y = remove_y

    def get_from_x(self):
        return self.from_x

    def get_from_y(self):
        return self.from_y

    def get_step_x(self):
        return self.step_x

    def get_step_y(self):
        return self.step_y

    def get_remove_x(self):
        return self.remove_x

    def get_remove_y(self):
        return self.remove_y

    def __repr__(self):
        return "{} {} {} {} {} {}".format(self.from_x, self.from_y, self.step_x, self.step_y, self.remove_x, self.remove_y)

    def __eq__(self, other):
        return self.step_x == other.step_x and self.step_y == other.step_y \
               and self.remove_x == other.remove_x and self.remove_y == other.remove_y

    def __ne__(self, other):
        return not self.__eq__(self, other)

    def __hash__(self):
        return self.step_y * SIDE ** 3 + self.step_x * SIDE ** 2 + self.remove_y * SIDE + self.remove_x


class IsolaMoveReader(MoveReader):
    def read(self):
        user = map(int, input("Enter space separated step_x step_y remove_x remove_y: ").split())
        x, y = state.get_player_cords(PLAYER_1)
        return IsolaMove(x, y, *user)


if __name__ == "__main__":
    state = IsolaState(SIDE, "__2__"
                             "_____"
                             "_____"
                             "_____"
                             "__1__")

    algorithm_1 = Minimax(PLAYER_1, PLAYER_2, 1)
    algorithm_2 = Minimax(PLAYER_2, PLAYER_1, 1)

    tester = Tester(state, algorithm_1, algorithm_2)
    tester.start()