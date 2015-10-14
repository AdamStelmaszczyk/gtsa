import pytest
from examples.isola import IsolaState
from gtsa import Minimax, MonteCarloTreeSearch


def test_isola_move():
    state = IsolaState(3, "###"
                          "#2#"
                          "#1_")
    for algorithm in [Minimax('1', '2'), MonteCarloTreeSearch('1', '2')]:
        assert algorithm.get_move(state) == (1, 2, 2, 2, 1, 2)


def test_isola_finish():
    state = IsolaState(3, "2#_"
                          "_#_"
                          "__1")
    for algorithm in [Minimax('1', '2'), MonteCarloTreeSearch('1', '2')]:
        move = algorithm.get_move(state)
        assert move[4] == 0 and move[5] == 1


def test_isola_not_lose():
    state = IsolaState(3, "___"
                          "2#1"
                          "_#_")
    for algorithm in [Minimax('1', '2'), MonteCarloTreeSearch('1', '2')]:
        move = algorithm.get_move(state)
        assert move[2] != 2 or move[3] != 2


def test_isola_terminal():
    state = IsolaState(3, "#2#"
                          "###"
                          "#1#")
    for algorithm in [Minimax('1', '2'), MonteCarloTreeSearch('1', '2')]:
        with pytest.raises(ValueError):
            algorithm.get_move(state)
