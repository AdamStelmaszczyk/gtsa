#include <assert.h>

#include "../examples/tic_tac_toe.cpp"

static const int MAX_TEST_SIMULATIONS = 5000;

template<class S, class M>
vector<shared_ptr<Algorithm<S, M>>> get_algorithms() {
    return {
        shared_ptr<Algorithm<S, M>>(new MonteCarloTreeSearch<S, M>(1, MAX_TEST_SIMULATIONS, true)),
        shared_ptr<Algorithm<S, M>>(new Minimax<S, M>()),
    };
}

void test_tic_tac_toe_finish() {
    TicTacToeState state = TicTacToeState("XX_"
                                          "_O_"
                                          "___");
    for (auto &algorithm : get_algorithms<TicTacToeState, TicTacToeMove>()) {
        auto move = algorithm->get_move(&state);
        assert(move == TicTacToeMove(2, 0));
    }
}

void test_tic_tac_toe_block() {
    TicTacToeState state = TicTacToeState("O__"
                                          "OX_"
                                          "___");
    for (auto &algorithm : get_algorithms<TicTacToeState, TicTacToeMove>()) {
        auto move = algorithm->get_move(&state);
        assert(move == TicTacToeMove(0, 2));
    }
}

void test_tic_tac_toe_block_2() {
    TicTacToeState state = TicTacToeState("X__"
                                          "OO_"
                                          "___");
    for (auto &algorithm : get_algorithms<TicTacToeState, TicTacToeMove>()) {
        auto move = algorithm->get_move(&state);
        assert(move == TicTacToeMove(2, 1));
    }
}

void test_tic_tac_toe_block_3() {
    TicTacToeState state = TicTacToeState("X__"
                                          "XO_"
                                          "O__");
    for (auto &algorithm : get_algorithms<TicTacToeState, TicTacToeMove>()) {
        auto move = algorithm->get_move(&state);
        assert(move == TicTacToeMove(2, 0));
    }
}

void test_tic_tac_toe_block_4() {
    TicTacToeState state = TicTacToeState("__X"
                                          "_OO"
                                          "__X");
    for (auto &algorithm : get_algorithms<TicTacToeState, TicTacToeMove>()) {
        auto move = algorithm->get_move(&state);
        assert(move == TicTacToeMove(0, 1));
    }
}

void test_tic_tac_toe_block_5() {
    TicTacToeState state = TicTacToeState("OX_"
                                          "_X_"
                                          "OOX");
    for (auto &algorithm : get_algorithms<TicTacToeState, TicTacToeMove>()) {
        auto move = algorithm->get_move(&state);
        assert(move == TicTacToeMove(0, 1));
    }
}

void test_tic_tac_toe_corner() {
    TicTacToeState state = TicTacToeState("___"
                                          "_O_"
                                          "___");
    for (auto &algorithm : get_algorithms<TicTacToeState, TicTacToeMove>()) {
        auto move = algorithm->get_move(&state);
        assert(move == TicTacToeMove(0, 0) || move == TicTacToeMove(0, 2) ||
               move == TicTacToeMove(2, 0) || move == TicTacToeMove(2, 2));
    }
}

void test_tic_tac_toe_terminal() {
    TicTacToeState state = TicTacToeState("XOX"
                                          "OOX"
                                          "OXO");
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
    Tester<TicTacToeState, TicTacToeMove> tester(&state, algorithms, 2, 2);
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