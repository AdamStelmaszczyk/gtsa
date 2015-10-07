import pytest
from examples.isola import IsolaState, IsolaMove
from gtsa.gtsa import Minimax, MonteCarloTreeSearch


def test_isola_move():
    state = IsolaState(3, "###"
                          "#2#"
                          "#1_")
    for algorithm in [Minimax('1', '2'), MonteCarloTreeSearch('1', '2')]:
        assert algorithm.get_move(state) == IsolaMove(1, 2, 2, 2, 1, 2)


def test_isola_finish():
    state = IsolaState(3, "2#_"
                          "_#_"
                          "__1")
    for algorithm in [Minimax('1', '2'), MonteCarloTreeSearch('1', '2')]:
        move = algorithm.get_move(state)
        assert move.get_remove_x() == 0 and move.get_remove_y() == 1


def test_isola_not_lose():
    state = IsolaState(3, "___"
                          "2#1"
                          "_#_")
    for algorithm in [Minimax('1', '2'), MonteCarloTreeSearch('1', '2')]:
        move = algorithm.get_move(state)
        assert move.get_step_x() != 2 and move.get_step_y() != 2


def test_isola_not_lose_2():
    state = IsolaState(3, "###"
                          "___"
                          "12_")
    for algorithm in [Minimax('1', '2'),
                      MonteCarloTreeSearch('1', '2', max_simulations=1000)]:
        move = algorithm.get_move(state)
        assert move != IsolaMove(0, 2, 0, 1, 1, 1)


def test_isola_terminal():
    state = IsolaState(3, "#2#"
                          "###"
                          "#1#")
    for algorithm in [Minimax('1', '2'), MonteCarloTreeSearch('1', '2')]:
        with pytest.raises(ValueError):
            algorithm.get_move(state)
