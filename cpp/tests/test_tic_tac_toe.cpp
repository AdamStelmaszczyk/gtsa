#include <assert.h>

#include "../examples/tic_tac_toe.cpp"

static const int MAX_TEST_SIMULATIONS = 1000;

template<class S, class M>
vector<shared_ptr<Algorithm<S, M>>> get_algorithms() {
    return {
        shared_ptr<Algorithm<S, M>>(new MonteCarloTreeSearch<S, M>(1, MAX_TEST_SIMULATIONS, true)),
        shared_ptr<Algorithm<S, M>>(new Minimax<S, M>()),
    };
}

void test_tic_tac_toe_finish() {
    TicTacToeState state = TicTacToeState("11_"
                                          "_2_"
                                          "___");
    for (auto &algorithm : get_algorithms<TicTacToeState, TicTacToeMove>()) {
        auto move = algorithm->get_move(&state);
        assert(move == TicTacToeMove(2, 0));
    }
}

void test_tic_tac_toe_block() {
    TicTacToeState state = TicTacToeState("2__"
                                          "21_"
                                          "___");
    for (auto &algorithm : get_algorithms<TicTacToeState, TicTacToeMove>()) {
        auto move = algorithm->get_move(&state);
        assert(move == TicTacToeMove(0, 2));
    }
}

void test_tic_tac_toe_block_2() {
    TicTacToeState state = TicTacToeState("1__"
                                          "22_"
                                          "___");
    for (auto &algorithm : get_algorithms<TicTacToeState, TicTacToeMove>()) {
        auto move = algorithm->get_move(&state);
        assert(move == TicTacToeMove(2, 1));
    }
}

void test_tic_tac_toe_block_3() {
    TicTacToeState state = TicTacToeState("1__"
                                          "12_"
                                          "2__");
    for (auto &algorithm : get_algorithms<TicTacToeState, TicTacToeMove>()) {
        auto move = algorithm->get_move(&state);
        assert(move == TicTacToeMove(2, 0));
    }
}

void test_tic_tac_toe_block_4() {
    TicTacToeState state = TicTacToeState("__1"
                                          "_22"
                                          "__1");
    for (auto &algorithm : get_algorithms<TicTacToeState, TicTacToeMove>()) {
        auto move = algorithm->get_move(&state);
        assert(move == TicTacToeMove(0, 1));
    }
}

void test_tic_tac_toe_block_5() {
    TicTacToeState state = TicTacToeState("21_"
                                          "_1_"
                                          "221");
    for (auto &algorithm : get_algorithms<TicTacToeState, TicTacToeMove>()) {
        auto move = algorithm->get_move(&state);
        assert(move == TicTacToeMove(0, 1));
    }
}

void test_tic_tac_toe_corner() {
    TicTacToeState state = TicTacToeState("___"
                                          "_2_"
                                          "___");
    for (auto &algorithm : get_algorithms<TicTacToeState, TicTacToeMove>()) {
        auto move = algorithm->get_move(&state);
        assert(move == TicTacToeMove(0, 0) || move == TicTacToeMove(0, 2) ||
               move == TicTacToeMove(2, 0) || move == TicTacToeMove(2, 2));
    }
}

void test_tic_tac_toe_terminal() {
    TicTacToeState state = TicTacToeState("121"
                                          "221"
                                          "212");
    for (auto &algorithm : get_algorithms<TicTacToeState, TicTacToeMove>()) {
        bool exception_thrown = false;
        try {
            algorithm->get_move(&state);
        } catch (invalid_argument &) {
            exception_thrown = true;
        }
        assert(exception_thrown);
    }
}

void test_tic_tac_toe_draw() {
    TicTacToeState state = TicTacToeState("___"
                                          "___"
                                          "___");
    auto algorithms = get_algorithms<TicTacToeState, TicTacToeMove>();
    assert(algorithms.size() == 2);
    Tester<TicTacToeState, TicTacToeMove> tester(&state, algorithms, 2);
    auto outcome_counts = tester.start();
    assert(outcome_counts.draws == 2);
}

int main() {
    test_tic_tac_toe_finish();
    test_tic_tac_toe_block();
    test_tic_tac_toe_block_2();
    test_tic_tac_toe_block_3();
    test_tic_tac_toe_block_4();
    test_tic_tac_toe_block_5();
    test_tic_tac_toe_corner();
    test_tic_tac_toe_terminal();
    test_tic_tac_toe_draw();
    return 0;
}