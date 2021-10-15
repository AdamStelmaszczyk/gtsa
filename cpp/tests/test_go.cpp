#include "../examples/go.cpp"

static const int MAX_TEST_SIMULATIONS = 1000;

void test_get_stones() {
    auto state = GoState("____1"
                         "1__2_"
                         "_22__"
                         "___1_"
                         "222__"
                         );
    assert(state.get_stones(0) == 3);
    assert(state.get_stones(1) == 6);
}

void test_get_area() {
    auto state = GoState("_1_1_"
                         "_1_21"
                         "1_2_2"
                         "11_2_"
                         "____2"
                         );
    assert(state.get_area(0) == 3);
    assert(state.get_area(1) == 2);
}

void test_make_move() {
    auto state = GoState("2____"
                         "1____"
                         "_____"
                         "___2_"
                         "_____");
    auto copy = state.clone();
    GoMove move = {1, 0};
    state.make_move(move);
    assert(state.get_area(0) == 1);
    state.undo_move(move);
    assert(state == copy);
}

void test_make_move_2() {
    auto state = GoState("_21__"
                         "1221_"
                         "_11__"
                         "_____"
                         "___2_");
    auto copy = state.clone();
    GoMove move = {0, 0};
    state.make_move(move);
    assert(state.get_area(0) == 3);
    state.undo_move(move);
    assert(state == copy);
}

void test_make_move_3() {
    auto state = GoState("_2___"
                         "12___"
                         "2____"
                         "_____"
                         "_____");
    auto copy = state.clone();
    GoMove move = {0, 0};
    state.make_move(move);
    assert(state.get_stones(0) == 0);
    state.undo_move(move);
    assert(state == copy);
}

void test_ko() {
    auto state = GoState("_21__"
                         "2_21_"
                         "_21__"
                         "_____"
                         "_____");
    state.make_move({1, 1});
    auto moves = state.get_legal_moves();
    assert(find(moves.begin(), moves.end(), GoMove(2, 1)) == moves.end());
}

void test_suicide() {
    auto state = GoState("_2___"
                         "2_2__"
                         "_2___"
                         "_____"
                         "_____");
    auto moves = state.get_legal_moves();
    assert(find(moves.begin(), moves.end(), GoMove(1, 1)) == moves.end());
}

void test_capture() {
    auto state = GoState("__1__"
                         "__212"
                         "_1212"
                         "_1212"
                         "__12_");
    auto algorithm = MonteCarloTreeSearch<GoState, GoMove>(1, MAX_TEST_SIMULATIONS);
    auto move = algorithm.get_move(&state);
    assert(move == GoMove(1, 1));
}

int main() {
    test_get_stones();
    test_get_area();
    test_make_move();
    test_make_move_2();
    test_make_move_3();
    test_ko();
    test_suicide();
    test_capture();
    return 0;
}