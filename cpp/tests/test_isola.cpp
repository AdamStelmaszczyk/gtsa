#include <assert.h>

#include "../examples/isola.cpp"

template<class S, class M>
vector<Algorithm<S, M> *> get_algorithms() {
    return {
        new MonteCarloTreeSearch<S, M>(),
        new Minimax<S, M>(),
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
        delete algorithm;
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
        delete algorithm;
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
        delete algorithm;
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
        delete algorithm;
        assert(move == IsolaMove(1, 0, 2, 1, 1, 0));
    }
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
        delete algorithm;
        assert(exception_thrown);
    }
}

int main() {
    test_isola_move();
    test_isola_finish();
    test_isola_not_lose();
    test_isola_crowded();
    test_isola_terminal();
    return 0;
}