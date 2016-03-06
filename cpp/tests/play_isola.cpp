#include "../examples/isola.cpp"

int main() {
    IsolaState state = IsolaState("___2___"
                                  "_______"
                                  "_______"
                                  "_______"
                                  "_______"
                                  "_______"
                                  "___1___");

    auto a = Minimax<IsolaState, IsolaMove>(1, true);
    auto b = MonteCarloTreeSearch<IsolaState, IsolaMove>(1, true);

    Tester<IsolaState, IsolaMove> tester(&state, a, b);
    tester.start();

    return 0;
}