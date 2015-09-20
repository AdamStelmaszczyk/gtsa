class Tester(object):

    def __init__(self, state, algorithm_1, algorithm_2):
        self.state = state
        self.algorithm_1 = algorithm_1
        self.player_1 = algorithm_1.get_current_player()
        self.algorithm_2 = algorithm_2
        self.player_2 = algorithm_2.get_current_player()

    def start(self):
        print self.state
        while True:
            move = self.algorithm_1.get_move(self.state)
            self.state.make_move(move, self.player_1)
            print self.state
            if self.state.is_terminal(self.player_1, self.player_2):
                break

            move = self.algorithm_2.get_move(self.state)
            self.state.make_move(move, self.player_2)
            print self.state
            if self.state.is_terminal(self.player_2, self.player_1):
                break