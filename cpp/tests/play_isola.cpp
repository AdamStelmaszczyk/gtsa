#include "../examples/isola.cpp"

int main() {
    IsolaState state = IsolaState("___2___"
                                  "_______"
                                  "_______"
                                  "_______"
                                  "_______"
                                  "_______"
                                  "___1___");

    Minimax<IsolaState, IsolaMove> a(0.1, 30);
    Minimax<IsolaState, IsolaMove> b(0.1);

    Tester<IsolaState, IsolaMove> tester(&state, a, b);
    tester.start();

    return 0;
}