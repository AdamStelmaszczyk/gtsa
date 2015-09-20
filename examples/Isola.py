from Algorithm import State, Move, Minimax
from Tester import Tester


SIDE = 7
PLAYER_1 = '1'
PLAYER_2 = '2'
EMPTY = '_'
REMOVED = '#'


class IsolaState(State):
    def __init__(self, side):
        super(IsolaState, self).__init__()
        self.side = side
        self.board = [[' ' for _ in xrange(side)] for _ in xrange(side)]

    def get_goodness(self, current_player, next_player):
        raise NotImplementedError()

    def get_legal_moves(self, player):
        step_moves = []
        player_cords = self.get_player_cords(player)
        for dy in xrange(-1, 2):
            for dx in xrange(-1, 2):
                x = player_cords[0] + dx
                y = player_cords[1] + dy
                if x < 0 or x >= self.side or y < 0 or y >= self.side:
                    continue
                if self.board[y][x] == '0':
                    step_moves.append((x, y))

        remove_moves = []
        for y in xrange(self.side):
            for x in xrange(self.side):
                if self.board[y][x] == '0':
                    remove_moves.append((x, y))

        legal_moves = []
        for step_move in step_moves:
            for remove_move in remove_moves:
                if step_move != remove_move:
                    legal_moves.append(IsolaMove(step_move[0], step_move[1], remove_move[0], remove_move[1]))
        return legal_moves

    def make_move(self, move, player):
        raise NotImplementedError()

    def undo_move(self, move):
        raise NotImplementedError()

    def set_state(self, string):
        correct_length = self.side ** 2
        if len(string) != correct_length:
            raise ValueError("Initialization string length must be {}".format(correct_length))
        for i, char in enumerate(string):
            x = i % self.side
            y = i / self.side
            self.board[y][x] = char

    def is_terminal(self, current_player, next_player):
        raise NotImplementedError()

    def get_player_cords(self, player):
        for y in xrange(self.side):
            for x in xrange(self.side):
                if self.board[y][x] == player:
                    return x, y

    def __repr__(self):
        return str(self.board)


class IsolaMove(Move):
    def __init__(self, step_x, step_y, remove_x, remove_y):
        self.step_x = step_x
        self.step_y = step_y
        self.remove_x = remove_x
        self.remove_y = remove_y

    def __repr__(self):
        return "{} {} {} {}".format(self.step_x, self.step_y, self.remove_x, self.remove_y)

    def __eq__(self, other):
        return self.step_x == other.step_x and self.step_y == other.step_y \
               and self.remove_x == other.remove_x and self.remove_y == other.remove_y


state = IsolaState(SIDE)
state.set_state("___2_________________________________________1___")

algorithm = Minimax(PLAYER_1, PLAYER_2, 1)
print algorithm.get_move(state)