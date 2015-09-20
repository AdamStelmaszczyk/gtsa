class Algorithm(object):
    def __init__(self, our_symbol, enemy_symbol):
        self.our_symbol = our_symbol
        self.enemy_symbol = enemy_symbol

    def get_current_player(self):
        return self.our_symbol

    def get_next_player(self):
        return self.enemy_symbol

    def get_move(self, state):
        raise NotImplementedError("Implement get_move in Algorithm subclass")

    def __repr__(self):
        return "{} {}".format(self.our_symbol, self.enemy_symbol)


class Minimax(Algorithm):
    def __init__(self, our_symbol, enemy_symbol, max_depth):
        super(Minimax, self).__init__(our_symbol, enemy_symbol)
        self.max_depth = max_depth

    def get_move(self, state):
        legal_moves = state.get_legal_moves()
        if not legal_moves:
            raise ValueError("Given state is terminal")
        best_move = None
        best_goodness = float('-inf')
        for move in legal_moves:
            state.make_move(move, self.our_symbol)
            goodness = self._minimax(state, self.max_depth, self.enemy_symbol)
            state.undo_move(move)
            if best_goodness < goodness:
                best_goodness = goodness
                best_move = move
        return best_move

    def _minimax(self, state, depth, our_symbol):
        legal_moves = state.get_legal_moves()
        if depth <= 0 or state.is_terminal(self.our_symbol, self.enemy_symbol):
            return state.get_goodness(self.our_symbol, self.enemy_symbol)
        if our_symbol == self.our_symbol:
            best_goodness = float('-inf')
            for move in legal_moves:
                state.make_move(move, our_symbol)
                goodness = self._minimax(state, depth - 1, self.enemy_symbol)
                state.undo_move(move)
                if best_goodness < goodness:
                    best_goodness = goodness
        else:
            best_goodness = float('inf')
            for move in legal_moves:
                state.make_move(move, our_symbol)
                goodness = self._minimax(state, depth - 1, self.our_symbol)
                state.undo_move(move)
                if best_goodness > goodness:
                    best_goodness = goodness
        return best_goodness


class State(object):
    def get_goodness(self, current_player, next_player):
        raise NotImplementedError("Implement get_goodness in State subclass")

    def get_legal_moves(self):
        raise NotImplementedError("Implement get_legal_moves in State subclass")

    def make_move(self, move, player):
        raise NotImplementedError("Implement make_move in State subclass")

    def undo_move(self, move):
        raise NotImplementedError("Implement undo_move in State subclass")

    def set_state(self, string):
        raise NotImplementedError("Implement set_state in State subclass")

    def is_terminal(self, current_player, next_player):
        raise NotImplementedError("Implement is_terminal in State subclass")

    def __repr__(self):
        raise NotImplementedError("Implement __repr__ in State subclass")


class Move(object):
    def __repr__(self):
        raise NotImplementedError("Implement __repr__ in Move subclass")

    def __eq__(self, other):
        raise NotImplementedError("Implement __eq__ in Move subclass")


class MoveReader(object):
    def read(self):
        raise NotImplementedError("Implement read in MoveReader subclass")