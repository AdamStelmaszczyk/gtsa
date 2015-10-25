#include <assert.h>

#include "../examples/tic_tac_toe.cpp"

template<class S, class M>
vector<Algorithm<S, M> *> get_algorithms() {
    return {
        new Minimax<S, M>('X', 'O'),
        new MonteCarloTreeSearch<S, M>('X', 'O'),
    };
}

void test_tic_tac_toe_finish() {
    string init_string =
        "XX_"
        "_O_"
        "___";
    TicTacToeState state = TicTacToeState(init_string);
    for (const auto &algorithm : get_algorithms<TicTacToeState, TicTacToeMove>()) {
        const auto move = algorithm->get_move(&state);
        delete algorithm;
        assert(move == TicTacToeMove(2, 0));
    }
}

void test_tic_tac_toe_block() {
    string init_string =
        "O__"
        "O__"
        "___";
    TicTacToeState state = TicTacToeState(init_string);
    for (const auto &algorithm : get_algorithms<TicTacToeState, TicTacToeMove>()) {
        const auto move = algorithm->get_move(&state);
        delete algorithm;
        assert(move == TicTacToeMove(0, 2));
    }
}

void test_tic_tac_toe_block_2() {
    string init_string =
        "X__"
        "OO_"
        "___";
    TicTacToeState state = TicTacToeState(init_string);
    for (const auto &algorithm : get_algorithms<TicTacToeState, TicTacToeMove>()) {
        const auto move = algorithm->get_move(&state);
        delete algorithm;
        assert(move == TicTacToeMove(2, 1));
    }
}

void test_tic_tac_toe_corner() {
    string init_string =
        "___"
        "_O_"
        "___";
    TicTacToeState state = TicTacToeState(init_string);
    for (const auto &algorithm : get_algorithms<TicTacToeState, TicTacToeMove>()) {
        const auto move = algorithm->get_move(&state);
        delete algorithm;
        assert(move == TicTacToeMove(0, 0) || move == TicTacToeMove(0, 2) ||
               move == TicTacToeMove(2, 0) || move == TicTacToeMove(2, 2));
    }
}

void test_tic_tac_toe_terminal() {
    string init_string =
        "XOX"
        "OOX"
        "OXO";
    TicTacToeState state = TicTacToeState(init_string);
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