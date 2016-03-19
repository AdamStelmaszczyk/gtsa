#include "../examples/isola.cpp"

int main() {
    IsolaState state = IsolaState("___2___"
                                  "_______"
                                  "_______"
                                  "_______"
                                  "_______"
                                  "_______"
                                  "___1___");

    auto a = MonteCarloTreeSearch<IsolaState, IsolaMove>(0.1, false);
    auto b = Minimax<IsolaState, IsolaMove>(0.1, false);

    Tester<IsolaState, IsolaMove> tester(&state, a, b, 100, false);
    tester.start();

    return 0;
}