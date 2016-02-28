#include "../examples/isola.cpp"

int main() {
    IsolaState state = IsolaState("___2___"
                                  "_______"
                                  "_______"
                                  "_______"
                                  "_______"
                                  "_______"
                                  "___1___");
    auto a = new Minimax<IsolaState, IsolaMove>(1, true);
    auto b = new MonteCarloTreeSearch<IsolaState, IsolaMove>(1, true);
    Tester<IsolaState, IsolaMove> tester(&state, *a, *b);
    tester.start();
    return 0;
}