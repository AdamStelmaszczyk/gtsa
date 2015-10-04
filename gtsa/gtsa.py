from __future__ import print_function
import time


class Algorithm(object):
    def __init__(self, our_symbol, enemy_symbol):
        self.our_symbol = our_symbol
        self.enemy_symbol = enemy_symbol

    def get_our_symbol(self):
        return self.our_symbol

    def get_enemy_symbol(self):
        return self.enemy_symbol

    def get_opposite_player(self, player):
        return self.our_symbol if player == self.enemy_symbol \
            else self.enemy_symbol

    def get_move(self, state):
        raise NotImplementedError("Implement get_move in Algorithm subclass")

    def __repr__(self):
        return "{} {}".format(self.our_symbol, self.enemy_symbol)


class Human(Algorithm):
    def __init__(self, our_symbol, enemy_symbol, move_reader):
        super(Human, self).__init__(our_symbol, enemy_symbol)
        self.move_reader = move_reader

    def get_move(self, state):
        legal_moves = state.get_legal_moves(self.our_symbol)
        if not legal_moves:
            raise ValueError("Given state is terminal")
        move = self.move_reader.read()
        if move not in legal_moves:
            raise ValueError("Illegal move")
        return move


class Minimax(Algorithm):
    def __init__(self, our_symbol, enemy_symbol, max_depth=10):
        super(Minimax, self).__init__(our_symbol, enemy_symbol)
        self.max_depth = max_depth

    def get_move(self, state):
        if state.is_terminal(self.our_symbol):
            raise ValueError("Given state is terminal: {}".format(state))
        _, best_move = self._minimax(state,
                                     self.max_depth,
                                     float('-inf'),
                                     float('inf'),
                                     self.our_symbol)
        return best_move

    def _minimax(self, state, depth, alpha, beta, analyzed_player):
        legal_moves = state.get_legal_moves(analyzed_player)
        if depth <= 0 or state.is_terminal(analyzed_player):
            return state.get_goodness(self.our_symbol), None
        best_move = None
        if analyzed_player == self.our_symbol:
            best_goodness = float('-inf')
            for move in legal_moves:
                state.make_move(move, analyzed_player)
                goodness, _ = self._minimax(state,
                                            depth - 1,
                                            alpha,
                                            beta,
                                            self.enemy_symbol)
                state.undo_move(move, analyzed_player)
                if best_goodness < goodness:
                    best_goodness = goodness
                    best_move = move
                alpha = max(alpha, best_goodness)
                if beta <= alpha:
                    break
        else:
            best_goodness = float('inf')
            for move in legal_moves:
                state.make_move(move, analyzed_player)
                goodness, _ = self._minimax(state,
                                            depth - 1,
                                            alpha,
                                            beta,
                                            self.our_symbol)
                state.undo_move(move, analyzed_player)
                if best_goodness > goodness:
                    best_goodness = goodness
                    best_move = move
                beta = min(beta, best_goodness)
                if beta <= alpha:
                    break
        return best_goodness, best_move


class State(object):
    def get_goodness(self, current_player):
        raise NotImplementedError("Implement get_goodness in State subclass")

    def get_legal_moves(self, player):
        raise NotImplementedError(
            "Implement get_legal_moves in State subclass")

    def make_move(self, move, player):
        raise NotImplementedError("Implement make_move in State subclass")

    def undo_move(self, move, player):
        raise NotImplementedError("Implement undo_move in State subclass")

    def is_terminal(self, player):
        raise NotImplementedError("Implement is_terminal in State subclass")

    def is_winner(self, player):
        raise NotImplementedError("Implement is_winner in State subclass")

    def __repr__(self):
        raise NotImplementedError("Implement __repr__ in State subclass")

    def __eq__(self, other):
        raise NotImplementedError("Implement __eq__ in State subclass")

    def __ne__(self, other):
        raise NotImplementedError("Implement __ne__ in State subclass")

    def __hash__(self):
        raise NotImplementedError("Implement __hash__ in State subclass")


class Move(object):
    def __repr__(self):
        raise NotImplementedError("Implement __repr__ in Move subclass")

    def __eq__(self, other):
        raise NotImplementedError("Implement __eq__ in Move subclass")

    def __ne__(self, other):
        raise NotImplementedError("Implement __ne__ in Move subclass")

    def __hash__(self):
        raise NotImplementedError("Implement __hash__ in State subclass")


class MoveReader(object):
    def __init__(self, state):
        self.state = state

    def read(self):
        raise NotImplementedError("Implement read in MoveReader subclass")


class Timer:
    def __init__(self):
        self.start = time.clock()

    def stop(self):
        seconds_elapsed = time.clock() - self.start
        print("{0:.2f}s".format(seconds_elapsed))


class Tester(object):
    def __init__(self, state, algorithm_1, algorithm_2):
        self.state = state
        self.algorithm_1 = algorithm_1
        self.player_1 = algorithm_1.get_our_symbol()
        self.algorithm_2 = algorithm_2
        self.player_2 = algorithm_2.get_our_symbol()

    def start(self):
        print(self.state)
        while True:
            if self.state.is_terminal(self.player_1):
                break
            timer = Timer()
            move = self.algorithm_1.get_move(self.state)
            timer.stop()
            self.state.make_move(move, self.player_1)
            print(self.state)

            if self.state.is_terminal(self.player_2):
                break
            timer = Timer()
            move = self.algorithm_2.get_move(self.state)
            timer.stop()
            self.state.make_move(move, self.player_2)
            print(self.state)
