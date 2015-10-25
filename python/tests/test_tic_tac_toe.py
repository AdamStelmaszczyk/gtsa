import pytest
from examples.tic_tac_toe import TicTacToeState
from gtsa import Minimax, MonteCarloTreeSearch


def test_tic_tac_toe_finish():
    state = TicTacToeState("XX_"
                           "_O_"
                           "___")
    for algorithm in [Minimax('X', 'O'), MonteCarloTreeSearch('X', 'O')]:
        assert algorithm.get_move(state) == (2, 0)


def test_tic_tac_toe_block():
    state = TicTacToeState("O__"
                           "O__"
                           "___")
    for algorithm in [Minimax('X', 'O'), MonteCarloTreeSearch('X', 'O')]:
        assert algorithm.get_move(state) == (0, 2)


def test_tic_tac_toe_block_2():
    state = TicTacToeState("X__"
                           "OO_"
                           "___")
    for algorithm in [Minimax('X', 'O'), MonteCarloTreeSearch('X', 'O')]:
        assert algorithm.get_move(state) == (2, 1)


def test_tic_tac_toe_corner():
    state = TicTacToeState("___"
                           "_O_"
                           "___")
    for algorithm in [Minimax('X', 'O'), MonteCarloTreeSearch('X', 'O')]:
        move = algorithm.get_move(state)
        assert move in [(0, 0), (2, 0), (0, 2), (2, 2)]


def test_tic_tac_toe_terminal():
    state = TicTacToeState("XOX"
                           "OOX"
                           "OXO")
    for algorithm in [Minimax('X', 'O'), MonteCarloTreeSearch('X', 'O')]:
        with pytest.raises(ValueError):
            algorithm.get_move(state)
