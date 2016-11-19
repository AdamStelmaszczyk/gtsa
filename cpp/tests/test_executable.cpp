#include "../examples/isola.cpp"

int main() {
    IsolaState state = IsolaState("___2___"
                                  "_______"
                                  "_______"
                                  "_______"
                                  "_______"
                                  "_______"
                                  "___1___");

    Minimax<IsolaState, IsolaMove> a(0.9, 30);
    Executable<IsolaState, IsolaMove> b("./tests/marten.o");

    Tester<IsolaState, IsolaMove> tester(&state, a, b);
    tester.start();

    return 0;
}