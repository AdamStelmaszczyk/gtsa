#include "../examples/isola.cpp"

int main() {
    IsolaState state = IsolaState("___2___"
                                  "_______"
                                  "_______"
                                  "_______"
                                  "_______"
                                  "_______"
                                  "___1___");

    Minimax<IsolaState, IsolaMove> a(0.1, 20);
    Human<IsolaState, IsolaMove> b;

    Tester<IsolaState, IsolaMove> tester(&state, a, b);
    tester.start();

    return 0;
}