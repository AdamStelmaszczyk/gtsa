#include "../examples/isola.cpp"

int main() {
    IsolaState state = IsolaState("___2___"
                                  "_______"
                                  "_______"
                                  "_______"
                                  "_______"
                                  "_______"
                                  "___1___");

    MonteCarloTreeSearch<IsolaState, IsolaMove> a(1);
//    Minimax<IsolaState, IsolaMove> b(0.1);
//
//    Tester<IsolaState, IsolaMove> tester(&state, a, b);
//    tester.start();
    auto move = a.get_move(&state);
    cout << a.read_log() << endl;

    return 0;
}