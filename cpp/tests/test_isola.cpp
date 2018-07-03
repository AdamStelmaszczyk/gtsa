#include <assert.h>

#include "../examples/isola.cpp"

static const int MAX_TEST_SIMULATIONS = 1000;

template<class S, class M>
vector<shared_ptr<Algorithm<S, M>>> get_algorithms() {
    return {
        shared_ptr<Algorithm<S, M>>(new MonteCarloTreeSearch<S, M>(1, MAX_TEST_SIMULATIONS)),
        shared_ptr<Algorithm<S, M>>(new Minimax<S, M>()),
    };
}

void test_isola_move() {
    IsolaState state = IsolaState("#######"
                                  "#2#####"
                                  "#1_####"
                                  "#######"
                                  "#######"
                                  "#######"
                                  "#######");
    for (auto &algorithm : get_algorithms<IsolaState, IsolaMove>()) {
        auto move = algorithm->get_move(&state);
        assert(move == IsolaMove(1, 2, 2, 2, 1, 2));
    }
}

void test_isola_finish() {
    IsolaState state = IsolaState("2#_####"
                                  "_#_####"
                                  "__1####"
                                  "#######"
                                  "#######"
                                  "#######"
                                  "#######");
    for (auto &algorithm : get_algorithms<IsolaState, IsolaMove>()) {
        auto move = algorithm->get_move(&state);
        assert(move.remove_x == 0 && move.remove_y == 1);
    }
}

void test_isola_not_lose() {
    IsolaState state = IsolaState("___####"
                                  "2#1####"
                                  "_#_####"
                                  "#######"
                                  "#######"
                                  "#######"
                                  "#######");
    for (auto &algorithm : get_algorithms<IsolaState, IsolaMove>()) {
        auto move = algorithm->get_move(&state);
        assert(move.step_x != 2 || move.step_y == 2);
    }
}

void test_isola_crowded() {
    IsolaState state = IsolaState("#12####"
                                  "_#_####"
                                  "#######"
                                  "#######"
                                  "#######"
                                  "#######"
                                  "#######");
    for (auto &algorithm : get_algorithms<IsolaState, IsolaMove>()) {
        auto move = algorithm->get_move(&state);
        assert(move == IsolaMove(1, 0, 2, 1, 1, 0));
    }
}

void test_isola_crowded_winner() {
    IsolaState state = IsolaState("##2####"
                                  "_#1####"
                                  "#######"
                                  "#######"
                                  "#######"
                                  "#######"
                                  "#######");
    assert(!state.is_winner(PLAYER_1));
    assert(state.is_winner(PLAYER_2));
}

void test_isola_terminal() {
    IsolaState state = IsolaState("###2###"
                                  "#######"
                                  "#######"
                                  "#######"
                                  "#######"
                                  "#######"
                                  "###1###");
    for (auto &algorithm : get_algorithms<IsolaState, IsolaMove>()) {
        bool exception_thrown = false;
        try {
            algorithm->get_move(&state);
        } catch (invalid_argument &) {
            exception_thrown = true;
        }
        assert(exception_thrown);
    }
}

void test_isola_make_and_undo() {
    IsolaState state = IsolaState("___2___"
                                  "_______"
                                  "_______"
                                  "_______"
                                  "_______"
                                  "_______"
                                  "___1___");
    auto copy = state.clone();
    auto move = IsolaMove(3, 6, 3, 5, 3, 6);
    state.make_move(move);
    state.undo_move(move);
    assert(state == copy);
}

void test_isola_make_and_undo_four_players() {
    IsolaState state = IsolaState("___3___"
                                  "_______"
                                  "_______"
                                  "2_____4"
                                  "_______"
                                  "_______"
                                  "___1___", 4);
    auto copy = state.clone();
    auto move = IsolaMove(3, 6, 3, 5, 3, 6);
    state.make_move(move);
    state.undo_move(move);
    assert(state == copy);
}

void test_isola_terminal_four_players() {
    IsolaState state = IsolaState("_______"
                                  "_#_####"
                                  "_2##3#_"
                                  "_#_41##"
                                  "_______"
                                  "_______"
                                  "_______", 4);
    assert(!state.is_terminal());
    state.player_to_move = '2';
    assert(!state.is_terminal());
    state.player_to_move = '3';
    assert(state.is_terminal());
    state.player_to_move = '4';
    assert(!state.is_terminal());

}

void test_get_remove_moves() {
    IsolaState state = IsolaState("___2___"
                                  "_______"
                                  "_______"
                                  "_______"
                                  "_______"
                                  "_______"
                                  "___1___");
    auto moves = state.get_remove_moves(6);
    assert(moves.size() == 6);
    using std::find;
    assert(find(moves.begin(), moves.end(), cords(3, 6)) != moves.end());
    assert(find(moves.begin(), moves.end(), cords(2, 0)) != moves.end());
    assert(find(moves.begin(), moves.end(), cords(4, 0)) != moves.end());
    assert(find(moves.begin(), moves.end(), cords(2, 1)) != moves.end());
    assert(find(moves.begin(), moves.end(), cords(3, 1)) != moves.end());
    assert(find(moves.begin(), moves.end(), cords(4, 1)) != moves.end());
}

int main() {
    test_isola_move();
    test_isola_finish();
    test_isola_not_lose();
    test_isola_crowded();
    test_isola_crowded_winner();
    test_isola_terminal();
    test_isola_make_and_undo();
    test_isola_make_and_undo_four_players();
    test_isola_terminal_four_players();
    test_get_remove_moves();
    return 0;
}