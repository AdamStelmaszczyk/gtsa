#include <assert.h>

#include "../examples/connect_four.cpp"

static const int MAX_TEST_SIMULATIONS = 1000;

template<class S, class M>
vector<shared_ptr<Algorithm<S, M>>> get_algorithms() {
    return {
        shared_ptr<Algorithm<S, M>>(new MonteCarloTreeSearch<S, M>()),
        shared_ptr<Algorithm<S, M>>(new Minimax<S, M>()),
    };
}

void test_is_winner() {
    ConnectFourState state = ConnectFourState("________"
                                              "________"
                                              "___1____"
                                              "___22_2_"
                                              "_2_21_12"
                                              "_1211_21"
                                              "22112_11");
    assert(state.is_winner(PLAYER_2));
}

void test_has_empty_space() {
    ConnectFourState state = ConnectFourState("21222112"
                                              "12111221"
                                              "11222112"
                                              "22111221"
                                              "21122122"
                                              "12112121"
                                              "21212112");
    assert(state.is_terminal());
}

void test_finish() {
    ConnectFourState state = ConnectFourState("___12___"
                                              "___11___"
                                              "___21___"
                                              "___21___"
                                              "__112_1_"
                                              "_222121_"
                                              "_2211212");
    for (auto &algorithm : get_algorithms<ConnectFourState, ConnectFourMove>()) {
        auto move = algorithm->get_move(&state);
        assert(move == ConnectFourMove(6));
    }
}

void test_block() {
    ConnectFourState state = ConnectFourState("________"
                                              "________"
                                              "________"
                                              "________"
                                              "__12____"
                                              "__212___"
                                              "221112__");
    for (auto &algorithm : get_algorithms<ConnectFourState, ConnectFourMove>()) {
        auto move = algorithm->get_move(&state);
        assert(move == ConnectFourMove(2));
    }
}

int main() {
    test_is_winner();
    test_has_empty_space();
    test_finish();
    test_block();
    return 0;
}