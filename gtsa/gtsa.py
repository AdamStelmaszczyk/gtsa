from __future__ import print_function
import time


class Algorithm(object):
    def __init__(self, our_symbol, enemy_symbol, show_progress=False):
        self.our_symbol = our_symbol
        self.enemy_symbol = enemy_symbol
        self.goodness_cache = {}
        self.show_progress = show_progress

    def get_current_player(self):
        return self.our_symbol

    def get_next_player(self):
        return self.enemy_symbol

    def get_move(self, state):
        raise NotImplementedError("Implement get_move in Algorithm subclass")

    def get_cached_goodness(self, state):
        key = hash(state)
        return self.goodness_cache.get(key, None)

    def set_cached_goodness(self, state, goodness):
        key = hash(state)
        self.goodness_cache[key] = goodness

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
    def __init__(self, our_symbol, enemy_symbol, max_depth, show_progress=False):
        super(Minimax, self).__init__(our_symbol, enemy_symbol, show_progress)
        self.max_depth = max_depth

    def get_move(self, state):
        legal_moves = state.get_legal_moves(self.our_symbol)
        if not legal_moves:
            raise ValueError("Given state is terminal: {}".format(state))
        best_move = None
        best_goodness = float('-inf')
        for i, move in enumerate(legal_moves):
            if self.show_progress:
                print("{}/{}".format(i, len(legal_moves)), end=' ', flush=True)
            state.make_move(move, self.our_symbol)
            goodness = self._minimax(state, self.max_depth, self.enemy_symbol)
            state.undo_move(move, self.our_symbol)
            if best_goodness < goodness:
                best_goodness = goodness
                best_move = move
        return best_move

    def _minimax(self, state, depth, analyzed_player):
        cached_goodness = self.get_cached_goodness(state)
        if cached_goodness:
            return cached_goodness
        legal_moves = state.get_legal_moves(analyzed_player)
        if depth <= 0 or state.is_terminal(self.our_symbol, self.enemy_symbol):
            return state.get_goodness(self.our_symbol, self.enemy_symbol)
        if analyzed_player == self.our_symbol:
            best_goodness = float('-inf')
            for move in legal_moves:
                state.make_move(move, analyzed_player)
                goodness = self._minimax(state, depth - 1, self.enemy_symbol)
                state.undo_move(move, analyzed_player)
                if best_goodness < goodness:
                    best_goodness = goodness
        else:
            best_goodness = float('inf')
            for move in legal_moves:
                state.make_move(move, analyzed_player)
                goodness = self._minimax(state, depth - 1, self.our_symbol)
                state.undo_move(move, analyzed_player)
                if best_goodness > goodness:
                    best_goodness = goodness
        self.set_cached_goodness(state, best_goodness)
        return best_goodness


class State(object):
    def get_goodness(self, current_player, next_player):
        raise NotImplementedError("Implement get_goodness in State subclass")

    def get_legal_moves(self, player):
        raise NotImplementedError("Implement get_legal_moves in State subclass")

    def make_move(self, move, player):
        raise NotImplementedError("Implement make_move in State subclass")

    def undo_move(self, move, player):
        raise NotImplementedError("Implement undo_move in State subclass")

    def is_terminal(self, current_player, next_player):
        raise NotImplementedError("Implement is_terminal in State subclass")

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
        self.player_1 = algorithm_1.get_current_player()
        self.algorithm_2 = algorithm_2
        self.player_2 = algorithm_2.get_current_player()

    def start(self):
        print(self.state)
        while True:
            if self.state.is_terminal(self.player_1, self.player_2):
                break
            timer = Timer()
            move = self.algorithm_1.get_move(self.state)
            timer.stop()
            self.state.make_move(move, self.player_1)
            print(self.state)

            if self.state.is_terminal(self.player_2, self.player_1):
                break
            timer = Timer()
            move = self.algorithm_2.get_move(self.state)
            timer.stop()
            self.state.make_move(move, self.player_2)
            print(self.state)
