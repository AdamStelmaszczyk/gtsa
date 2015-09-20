from Minimax import Minimax
from Move import Move
from State import State


class IsolaState(State):

    def __init__(self, side):
        super(IsolaState, self).__init__()
        self.side = side
        self.board = [[' ' for _ in xrange(side)] for _ in xrange(side)]

    def get_legal_moves(self):
        step_moves = []
        player_cords = self.get_player_cords()
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

    def get_player_cords(self):
        for y in xrange(self.side):
            for x in xrange(self.side):
                if self.board[y][x] == self.current_player:
                    return x, y

    def set_state(self, string):
        for i, char in enumerate(string):
            x = i % self.side
            y = i / self.side
            self.board[y][x] = char

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


state = IsolaState(7)
state.set_state("0002000000000000000000000000000000000000000001000")
print state.get_legal_moves()

algorithm = Minimax()
move = algorithm.get_move(state)