#include <assert.h>

#include "../examples/tic_tac_toe.hpp"

template<class S, class M>
vector<Algorithm<S, M> *> get_algorithms() {
    return {
        new Minimax<S, M>(PLAYER_1, PLAYER_2),
        new MonteCarloTreeSearch<S, M>(PLAYER_1, PLAYER_2),
    };
}

void test_tic_tac_toe_finish() {
    TicTacToeState state = TicTacToeState("XX_"
                                          "_O_"
                                          "___");
    for (const auto &algorithm : get_algorithms<TicTacToeState, TicTacToeMove>()) {
        const auto move = algorithm->get_move(&state);
        delete algorithm;
        assert(move == TicTacToeMove(2, 0));
    }
}

void test_tic_tac_toe_block() {
    TicTacToeState state = TicTacToeState("O__"
                                          "O__"
                                          "___");
    for (const auto &algorithm : get_algorithms<TicTacToeState, TicTacToeMove>()) {
        const auto move = algorithm->get_move(&state);
        delete algorithm;
        assert(move == TicTacToeMove(0, 2));
    }
}

void test_tic_tac_toe_block_2() {
    TicTacToeState state = TicTacToeState("X__"
                                          "OO_"
                                          "___");
    for (const auto &algorithm : get_algorithms<TicTacToeState, TicTacToeMove>()) {
        const auto move = algorithm->get_move(&state);
        delete algorithm;
        assert(move == TicTacToeMove(2, 1));
    }
}

void test_tic_tac_toe_corner() {
    TicTacToeState state = TicTacToeState("___"
                                          "_O_"
                                          "___");
    for (const auto &algorithm : get_algorithms<TicTacToeState, TicTacToeMove>()) {
        const auto move = algorithm->get_move(&state);
        delete algorithm;
        assert(move == TicTacToeMove(0, 0) || move == TicTacToeMove(0, 2) ||
               move == TicTacToeMove(2, 0) || move == TicTacToeMove(2, 2));
    }
}

void test_tic_tac_toe_terminal() {
    TicTacToeState state = TicTacToeState("XOX"
                                          "OOX"
                                          "OXO");
    for (const auto &algorithm : get_algorithms<TicTacToeState, TicTacToeMove>()) {
        bool exception_thrown = false;
        try {
            algorithm->get_move(&state);
        } catch (invalid_argument &) {
            exception_thrown = true;
        }
        delete algorithm;
        assert(exception_thrown);
    }
}

int main() {
    test_tic_tac_toe_finish();
    test_tic_tac_toe_block();
    test_tic_tac_toe_block_2();
    test_tic_tac_toe_corner();
    test_tic_tac_toe_terminal();
    return 0;
}