#include "../examples/isola.cpp"

int main() {
    IsolaState state = IsolaState("_2_"
                                  "___"
                                  "_1_");
    auto a = new Minimax<IsolaState, IsolaMove>(PLAYER_1, PLAYER_2);
    auto b = new MonteCarloTreeSearch<IsolaState, IsolaMove>(PLAYER_2, PLAYER_1);
    Tester<IsolaState, IsolaMove> tester(&state, *a, *b);
    tester.start();
    return 0;
}