import pytest
from examples.isola import IsolaState, IsolaMove
from gtsa.gtsa import Minimax


def test_isola_move():
    state = IsolaState(3, "###"
                          "#2#"
                          "#1_")
    algorithm = Minimax('1', '2', 1)
    assert algorithm.get_move(state) == IsolaMove(1, 2, 2, 2, 1, 2)


def test_isola_finish():
    state = IsolaState(3, "2#_"
                          "_#_"
                          "__1")
    algorithm = Minimax('1', '2', 1)
    move = algorithm.get_move(state)
    assert move.get_remove_x() == 0 and move.get_remove_y() == 1


def test_isola_not_lose():
    state = IsolaState(3, "___"
                          "2#1"
                          "_#_")
    algorithm = Minimax('1', '2', 1)
    move = algorithm.get_move(state)
    assert move.get_step_x() != 2 and move.get_step_y() != 2


def test_isola_terminal():
    state = IsolaState(3, "#2#"
                          "###"
                          "#1#")
    algorithm = Minimax('1', '2', 1)
    with pytest.raises(ValueError):
        algorithm.get_move(state)
