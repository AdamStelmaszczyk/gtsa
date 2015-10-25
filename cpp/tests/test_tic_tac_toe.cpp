#include <assert.h>
#include "../examples/tic_tac_toe.cpp"

void test_tic_tac_toe_finish() {
    string init_string =
            "XX_"
            "_O_"
            "___";
    TicTacToeState state = TicTacToeState(3, init_string);
    auto algorithm = MonteCarloTreeSearch<TicTacToeState, TicTacToeMove>('X', 'O');
    auto move = algorithm.get_move(&state);
    assert(move == TicTacToeMove(2, 0));
}

int main() {
    test_tic_tac_toe_finish();
    return 0;
}