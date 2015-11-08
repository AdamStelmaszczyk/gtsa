from __future__ import print_function
import random
import time
import math


EPSILON = 0.01
SQRT_2 = math.sqrt(2)


def get_random_from_generator(generator):
    results = [_ for _ in generator]
    return random.choice(results)


def get_class_name(algorithm):
    return type(algorithm).__name__


class Algorithm(object):
    def __init__(self, our_symbol, enemy_symbol):
        self.our_symbol = our_symbol
        self.enemy_symbol = enemy_symbol

    def get_opposite_player(self, player):
        return self.our_symbol if player == self.enemy_symbol \
            else self.enemy_symbol

    def get_move(self, state):
        raise NotImplementedError("Implement get_move in Algorithm subclass")

    def __repr__(self):
        return "{} {}".format(self.our_symbol, self.enemy_symbol)


class Human(Algorithm):
    def __init__(self, our_symbol, enemy_symbol, read_move_function):
        super(Human, self).__init__(our_symbol, enemy_symbol)
        self.read_move_function = read_move_function

    def get_move(self, state):
        legal_moves = [_ for _ in state.get_legal_moves(self.our_symbol)]
        if not legal_moves:
            raise ValueError("Given state is terminal:\n{}".format(state))
        while True:
            move = self.read_move_function()
            if move in legal_moves:
                return move
            else:
                print("Move {} is not legal".format(move))


class Minimax(Algorithm):
    def __init__(self,
                 our_symbol,
                 enemy_symbol,
                 max_seconds=1,
                 verbose=False):
        super(Minimax, self).__init__(our_symbol, enemy_symbol)
        self.max_seconds = max_seconds
        self.verbose = verbose
        self.timer = None
        self.max_depth = None

    def get_move(self, state):
        if state.is_terminal(self.our_symbol):
            raise ValueError("Given state is terminal:\n{}".format(state))
        self.timer = Timer()
        self.max_depth = 1
        best_goodness = float('-inf')
        best_move = None
        best_at_depth = 1
        while self.timer.seconds_elapsed() < self.max_seconds and \
                self.max_depth < 100:
            goodness, move = self._minimax(
                state,
                0,
                float('-inf'),
                float('inf'),
                self.our_symbol,
            )
            if self.verbose:
                print("goodness: {} at max_depth: {}".format(
                    goodness,
                    self.max_depth,
                ))
            if best_goodness <= goodness:
                best_goodness = goodness
                best_move = move
                best_at_depth = self.max_depth
            self.max_depth += 2
        if self.verbose:
            print("best_goodness: {} at max_depth: {}".format(
                best_goodness,
                best_at_depth,
            ))
        return best_move

    def _minimax(self, state, depth, alpha, beta, analyzed_player):
        if depth >= self.max_depth or state.is_terminal(analyzed_player) or \
                self.timer.seconds_elapsed() > self.max_seconds:
            return state.get_goodness(analyzed_player), None
        best_move = None
        legal_moves = state.get_legal_moves(analyzed_player)

        best_goodness = float('-inf')
        for move in legal_moves:
            state.make_move(move, analyzed_player)
            goodness = -self._minimax(
                state,
                depth + 1,
                -beta,
                -alpha,
                self.get_opposite_player(analyzed_player),
            )[0]
            print(goodness)
            state.undo_move(move, analyzed_player)
            if best_goodness < goodness:
                best_goodness = goodness
                best_move = move
            alpha = max(alpha, best_goodness)
            if best_goodness >= beta:
                break
        return best_goodness, best_move

class MonteCarloTreeSearch(Algorithm):
    def __init__(self, our_symbol,
                 enemy_symbol,
                 max_seconds=1,
                 max_simulations=10000,
                 verbose=False):
        super(MonteCarloTreeSearch, self).__init__(our_symbol, enemy_symbol)
        self.max_seconds = max_seconds
        self.max_simulations = max_simulations
        self.verbose = verbose

    def get_move(self, state):
        if state.is_terminal(self.our_symbol):
            raise ValueError("Given state is terminal:\n{}".format(state))
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
        # 1. Selection - find the most promising not expanded yet state
        current = state
        while current.has_children() and \
                not current.is_terminal(analyzed_player):
            current = current.select_child_by_uct()
            analyzed_player = self.get_opposite_player(analyzed_player)

        # 2. Expansion
        if not current.is_terminal(analyzed_player):
            current.expand(analyzed_player)
            current = current.select_child_by_uct()
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

    def has_children(self):
        return self.children

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


class Timer:
    def __init__(self):
        self.start = time.time()

    def seconds_elapsed(self):
        return time.time() - self.start

    def print_seconds_elapsed(self):
        print("{0:.1f}s".format(self.seconds_elapsed()))


class Tester(object):
    def __init__(
            self,
            state,
            algorithm_1,
            algorithm_2,
            matches=1,
            verbose=True,
    ):
        self.state = state
        self.algorithm_1 = algorithm_1
        self.player_1 = algorithm_1.our_symbol
        self.algorithm_2 = algorithm_2
        self.player_2 = algorithm_2.our_symbol
        self.matches = matches
        self.verbose = verbose
        self.algorithm_1_wins = None

    def handle_player(self, state, player, algorithm):
        if state.is_terminal(player):
            if state.is_winner(self.player_1):
                self.algorithm_1_wins += 1
            return True
        if self.verbose:
            print(algorithm.our_symbol, get_class_name(algorithm))
        timer = Timer()
        move = algorithm.get_move(state)
        if self.verbose:
            timer.print_seconds_elapsed()
        state.make_move(move, player)
        if self.verbose:
            print(state)
        return False

    def start(self):
        self.algorithm_1_wins = 0
        for i in range(self.matches):
            print("Match {}/{}".format(i + 1, self.matches))
            current_state = self.state.clone()
            if self.verbose:
                print(current_state)
            while True:
                end = self.handle_player(
                    current_state,
                    self.player_1,
                    self.algorithm_1,
                )
                if end:
                    break
                end = self.handle_player(
                    current_state,
                    self.player_2,
                    self.algorithm_2
                )
                if end:
                    break
        print("{} {} won {}/{} matches".format(
            self.player_1,
            get_class_name(self.algorithm_1),
            self.algorithm_1_wins,
            self.matches,
        ))
