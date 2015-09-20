from Algorithm import Algorithm


class Human(Algorithm):

    def __init__(self, our_symbol, enemy_symbol, move_reader):
        super(Human, self).__init__(our_symbol, enemy_symbol)
        self.move_reader = move_reader

    def get_move(self, state):
        legal_moves = state.get_legal_moves()
        if not legal_moves:
            raise ValueError("Given state is terminal")
        move = self.move_reader.read()
        if move not in legal_moves:
            raise ValueError("Illegal move")
        return move
