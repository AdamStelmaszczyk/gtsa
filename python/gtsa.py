from __future__ import print_function
import random
import time
import math


EPSILON = 0.01
SQRT_2 = math.sqrt(2)


def get_random_from_generator(generator):
    results = [_ for _ in generator]
    return random.choice(results)


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
        legal_moves = [_ for _ in state.get_legal_moves(self.our_symbol)]
        if not legal_moves:
            raise ValueError("Given state is terminal")
        while True:
            move = self.move_reader.read()
            if move in legal_moves:
                return move
            else:
                print("Move {} is not legal".format(move))


class Minimax(Algorithm):
    def __init__(self, our_symbol, enemy_symbol, max_seconds=10, max_depth=10):
        super(Minimax, self).__init__(our_symbol, enemy_symbol)
        self.max_seconds = max_seconds
        self.max_depth = max_depth
        self.timer = None

    def get_move(self, state):
        if state.is_terminal(self.our_symbol):
            raise ValueError("Given state is terminal: {}".format(state))
        self.timer = Timer()
        _, best_move = self._minimax(state,
                                     self.max_depth,
                                     float('-inf'),
                                     float('inf'),
                                     self.our_symbol)
        return best_move

    def _minimax(self, state, depth, alpha, beta, analyzed_player):
        if depth <= 0 or state.is_terminal(analyzed_player) or \
                self.timer.seconds_elapsed() > self.max_seconds:
            return state.get_goodness(self.our_symbol), None
        best_move = None
        legal_moves = state.get_legal_moves(analyzed_player)
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


class MonteCarloTreeSearch(Algorithm):
    def __init__(self, our_symbol,
                 enemy_symbol,
                 max_seconds=10,
                 max_simulations=500,
                 verbose=False):
        super(MonteCarloTreeSearch, self).__init__(our_symbol, enemy_symbol)
        self.max_seconds = max_seconds
        self.max_simulations = max_simulations
        self.verbose = verbose

    def get_move(self, state):
        if state.is_terminal(self.our_symbol):
            raise ValueError("Given state is terminal: {}".format(state))
        timer = Timer()
        state.remove_children()
        simulation = 0
        while simulation < self.max_simulations \
                and timer.seconds_elapsed() < self.max_seconds:
            self._monte_carlo_tree_search(state, self.our_symbol)
            simulation += 1
        if self.verbose:
            print("simulations: {} moves: {}".format(simulation,
                                                     len(state.children)))
            for child in state.children:
                print("move: {} trials: {} ratio: {:.1f}%".format(
                    child.move,
                    child.visits,
                    100 * child.get_win_ratio()))
        return state.select_child_by_ratio().move

    def _monte_carlo_tree_search(self, state, analyzed_player):
        # 1. Selection - find the most promising leaf to expand
        current = state
        while not current.is_leaf() and \
                not current.is_terminal(analyzed_player):
            current = current.select_child_by_uct()
            analyzed_player = self.get_opposite_player(analyzed_player)

        # 2. Expansion
        current.expand(analyzed_player)

        best_child = current.select_child_by_uct()
        if best_child:
            current = best_child
            analyzed_player = self.get_opposite_player(analyzed_player)

        # 3. Simulation
        result = self._simulate(current, analyzed_player)

        # 4. Propagation
        while current.parent:
            current.update_stats(result)
            current = current.parent
        current.update_stats(result)

    def _simulate(self, state, analyzed_player):
        opponent = self.get_opposite_player(analyzed_player)
        if state.is_terminal(analyzed_player) or state.is_terminal(opponent):
            if state.is_winner(analyzed_player):
                return 1 if analyzed_player == self.our_symbol else 0
            if state.is_winner(opponent):
                return 1 if opponent == self.our_symbol else 0
            return 0.5

        # If player has a winning move he makes it.
        for move in state.get_legal_moves(analyzed_player):
            state.make_move(move, analyzed_player)
            if state.is_winner(analyzed_player):
                state.undo_move(move, analyzed_player)
                return 1 if analyzed_player == self.our_symbol else 0
            state.undo_move(move, analyzed_player)

        # Otherwise random move.
        generator = state.get_legal_moves(analyzed_player)
        move = get_random_from_generator(generator)
        state.make_move(move, analyzed_player)
        result = self._simulate(state, opponent)
        state.undo_move(move, analyzed_player)
        return result


class State(object):
    def __init__(self):
        self.visits = 0
        self.score = 0
        self.player_who_moved = None
        self.move = None
        self.parent = None
        self.children = []

    def expand(self, player):
        current_children = []
        for move in self.get_legal_moves(player):
            child = self._create_child(move, player)
            if child.is_winner(player):
                # If player has a winning move he makes it.
                self.children.append(child)
                return
            current_children.append(child)
        self.children = current_children

    def _create_child(self, move, player_who_moved):
        child = self.clone()
        child.visits = 0
        child.score = 0
        child.player_who_moved = player_who_moved
        child.move = move
        child.parent = self
        child.children = []
        child.make_move(move, player_who_moved)
        return child

    def remove_children(self):
        self.children = []

    def update_stats(self, result):
        self.score += result
        self.visits += 1

    def is_leaf(self):
        return not self.children

    def select_child_by_ratio(self):
        best_child = None
        max_ratio = float('-inf')
        max_visits = float('-inf')
        for child in self.children:
            child_ratio = child.get_win_ratio()
            if max_ratio < child_ratio or \
                    (max_ratio == child_ratio and max_visits < child.visits):
                max_ratio = child_ratio
                max_visits = child.visits
                best_child = child
        return best_child

    def get_win_ratio(self):
        return self.score / (self.visits + EPSILON)

    def select_child_by_uct(self):
        best_child = None
        max_uct = float('-inf')
        for child in self.children:
            child_uct = child.get_uct_value()
            if max_uct < child_uct:
                max_uct = child_uct
                best_child = child
        return best_child

    def get_uct_value(self):
        return self.score / (self.visits + EPSILON) + \
            SQRT_2 * \
            math.sqrt(
                math.log(self.parent.visits + 1) / (self.visits + EPSILON)) \
            + random.random() * EPSILON

    def clone(self):
        raise NotImplementedError("Implement clone in State subclass")

    def get_goodness(self, current_player):
        raise NotImplementedError("Implement get_goodness in State subclass")

    def get_legal_moves(self, player):
        raise NotImplementedError(
            "Implement get_legal_moves in State subclass")

    def is_terminal(self, player):
        raise NotImplementedError("Implement is_terminal in State subclass")

    def is_winner(self, player):
        raise NotImplementedError("Implement is_winner in State subclass")

    def make_move(self, move, player):
        raise NotImplementedError("Implement make_move in State subclass")

    def undo_move(self, move, player):
        raise NotImplementedError("Implement undo_move in State subclass")

    def __repr__(self):
        raise NotImplementedError("Implement __repr__ in State subclass")


class MoveReader(object):
    def __init__(self, state):
        self.state = state

    def read(self):
        raise NotImplementedError("Implement read in MoveReader subclass")


class Timer:
    def __init__(self):
        self.start = time.clock()

    def seconds_elapsed(self):
        return time.clock() - self.start

    def print_seconds_elapsed(self):
        print("{0:.1f}s".format(self.seconds_elapsed()))


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
            print(type(self.algorithm_1).__name__)
            timer = Timer()
            move = self.algorithm_1.get_move(self.state)
            timer.print_seconds_elapsed()
            self.state.make_move(move, self.player_1)
            print(self.state)

            if self.state.is_terminal(self.player_2):
                break
            print(type(self.algorithm_2).__name__)
            timer = Timer()
            move = self.algorithm_2.get_move(self.state)
            timer.print_seconds_elapsed()
            self.state.make_move(move, self.player_2)
            print(self.state)
