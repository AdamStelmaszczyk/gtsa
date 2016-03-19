#include <assert.h>

#include "../examples/connect_four.cpp"

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

int main() {
    test_is_winner();
    test_has_empty_space();
    return 0;
}