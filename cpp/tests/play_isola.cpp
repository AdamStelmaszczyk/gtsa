#include "../examples/isola.cpp"

int main() {
    IsolaState state = IsolaState("___2___"
                                  "_______"
                                  "_______"
                                  "_______"
                                  "_______"
                                  "_______"
                                  "___1___");

    auto a = Minimax<IsolaState, IsolaMove>(0.1, false, 20);
    auto b = Minimax<IsolaState, IsolaMove>(0.1, false, 10);

    Tester<IsolaState, IsolaMove> tester(&state, a, b);
    tester.start();

    return 0;
}