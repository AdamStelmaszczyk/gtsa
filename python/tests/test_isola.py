import pytest
from examples.isola import IsolaState, PLAYER_1, PLAYER_2
from gtsa import Minimax, MonteCarloTreeSearch

MAX_SIMULATIONS = 500


def get_algorithms():
    return [
        Minimax(PLAYER_1, PLAYER_2),
        MonteCarloTreeSearch(
            PLAYER_1,
            PLAYER_2,
            max_simulations=MAX_SIMULATIONS,
        ),
    ]


def test_isola_move():
    state = IsolaState("###"
                       "#2#"
                       "#1_")
    for algorithm in get_algorithms():
        assert algorithm.get_move(state) == (1, 2, 2, 2, 1, 2)


def test_isola_finish():
    state = IsolaState("2#_"
                       "_#_"
                       "__1")
    for algorithm in get_algorithms():
        move = algorithm.get_move(state)
        assert move[4] == 0 and move[5] == 1


def test_isola_crowded():
    state = IsolaState("#12"
                       "_#_"
                       "##_")
    for algorithm in get_algorithms():
        assert algorithm.get_move(state) == (1, 0, 2, 1, 1, 0)


def test_isola_not_lose():
    state = IsolaState("___"
                       "2#1"
                       "_#_")
    for algorithm in get_algorithms():
        move = algorithm.get_move(state)
        assert move[2] != 2 or move[3] != 2


def test_isola_terminal():
    state = IsolaState("#2#"
                       "###"
                       "#1#")
    for algorithm in get_algorithms():
        with pytest.raises(ValueError):
            algorithm.get_move(state)
