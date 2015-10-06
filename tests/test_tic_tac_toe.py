import pytest
from examples.tic_tac_toe import TicTacToeState, TicTacToeMove
from gtsa.gtsa import Minimax, MonteCarloTreeSearch


def test_tic_tac_toe_finish():
    state = TicTacToeState(3, "XX_"
                              "_O_"
                              "___")
    for algorithm in [Minimax('X', 'O'), MonteCarloTreeSearch('X', 'O')]:
        assert algorithm.get_move(state) == TicTacToeMove(2, 0)


def test_tic_tac_toe_block():
    state = TicTacToeState(3, "O__"
                              "O__"
                              "___")
    for algorithm in [Minimax('X', 'O'), MonteCarloTreeSearch('X', 'O')]:
        assert algorithm.get_move(state) == TicTacToeMove(0, 2)


def test_tic_tac_toe_block_2():
    state = TicTacToeState(3, "X__"
                              "OO_"
                              "___")
    for algorithm in [Minimax('X', 'O'), MonteCarloTreeSearch('X', 'O')]:
        assert algorithm.get_move(state) == TicTacToeMove(2, 1)


def test_tic_tac_toe_corner():
    state = TicTacToeState(3, "___"
                              "_O_"
                              "___")
    for algorithm in [Minimax('X', 'O'), MonteCarloTreeSearch('X', 'O')]:
        move = algorithm.get_move(state)
        assert move in [TicTacToeMove(0, 0),
                        TicTacToeMove(2, 0),
                        TicTacToeMove(0, 2),
                        TicTacToeMove(2, 2)]


def test_tic_tac_toe_terminal():
    state = TicTacToeState(3, "XOX"
                              "OOX"
                              "OXO")
    for algorithm in [Minimax('X', 'O'), MonteCarloTreeSearch('X', 'O')]:
        with pytest.raises(ValueError):
            algorithm.get_move(state)
