import pytest
from examples.tic_tac_toe import TicTacToeState, TicTacToeMove
from gtsa.gtsa import Minimax


def test_tic_tac_toe_finish():
    state = TicTacToeState(3, "XX_"
                              "___"
                              "___")
    algorithm = Minimax('X', 'O', 10)
    assert algorithm.get_move(state) == TicTacToeMove(2, 0)

def test_tic_tac_toe_block():
    state = TicTacToeState(3, "O__"
                              "O__"
                              "___")
    algorithm = Minimax('X', 'O', 10)
    assert algorithm.get_move(state) == TicTacToeMove(0, 2)

def test_tic_tac_toe_block_2():
    state = TicTacToeState(3, "X__"
                              "OO_"
                              "___")
    algorithm = Minimax('X', 'O', 10)
    assert algorithm.get_move(state) == TicTacToeMove(2, 1)

def test_tic_tac_toe_terminal():
    state = TicTacToeState(3, "XOX"
                              "OOX"
                              "OXO")
    algorithm = Minimax('X', 'O', 10)
    with pytest.raises(ValueError):
        algorithm.get_move(state)